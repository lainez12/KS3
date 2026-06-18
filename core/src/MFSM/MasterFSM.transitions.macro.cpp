#include <QDebug>

#include <MFSM/MasterFSM.h>
#include <MFSM/interlocks.h>
#if defined(KUB_MODEL_8)
#include <Services/Contact/ContactService.h>
#endif
#include <utils.h>

namespace Kub3::MFSM
{

    // Helper: Safely extracts the last known posture from any state that holds one.
    // If the state doesn't track posture (like Booting), returns Unknown.
    static SystemPosture extractPosture(const SystemState &state)
    {
        const auto museum = overloadedCallable{
            [](const StateOperational &s) { return s.posture; }, [](const StateError &s) { return s.posture; },
            [](const StateEmergencyStop &s) { return s.posture; }, [](const auto &) { return SystemPosture{}; } // Defaults to Unknown
        };
        return std::visit(museum, state);
    }

    // ==========================================
    // LEVEL 1: MACRO TRANSITION DISPATCHER
    // ==========================================
    // Mathematically maps: (MacroState x Event) -> MacroState
    SystemState MasterFSM::processMacroTransition(const SystemState &currentState, const SystemEvent &event)
    {
        // GLOBAL INTERCEPTS (Absolute Priority)
        // --- Emergency stop
        if (auto *eStop = std::get_if<EvEmergencyStopTriggered>(&event))
        {
            qCritical() << "MFSM: EMERGENCY-STOP TRIGGERED:" << QString::fromStdString(eStop->reason);
            return StateEmergencyStop{.posture = extractPosture(currentState), .reason = eStop->reason};
        }
        // Power off
        if (std::holds_alternative<EvPowerOff>(event))
        {
            qWarning() << "MFSM: POWER OFF REQUESTED";
            if (std::holds_alternative<StateOperational>(currentState))
                return StatePreparePowerOff{}; // Triggers homing before power off
            return StatePowerOff{};            // Directly triggers poweroff
        }

        // STANDARD MACRO TRANSITIONS
        const auto museum = overloadedCallable(
            // --- BOOTING ---
            [&](const StateBooting &, const EvHardwareReady &) -> SystemState {
                return StateWaitingInitialization{};
            },
            [&](const StateBooting &, const EvHardwareError &e) -> SystemState {
                return StateError{.posture = {}, .message = e.reason};
            },

            // --- INITIALIZATION ---
            [&](const StateWaitingInitialization &, const CmdStartInitialization &) -> SystemState {
                return StateInitializing{};
            },
            [&](const StateInitializing &, const EvInitializationComplete &) -> SystemState {
                // Init successful: Provide the baseline posture and enter operational `Idle` state
                return StateOperational{
                    .posture  = {.wafer = WaferPosture::Homed, .mask = MaskPosture::Homed, .vision = VisionPosture::Homed},
                    .subState = StateIdle{},
                };
            },
            [&](const StateInitializing &, const EvServiceError &e) -> SystemState {
                return StateError{.posture = {}, .message = e.reason};
            },

            // --- OPERATIONAL (Delegation to Level 2) ---
            [&](const StateOperational &s, const EvServiceError &e) -> SystemState {
                // Dynamically extract expected posture if the active sub-state has one
                const ExpectedSystemPosture failedExpectation = std::visit(
                    overloadedCallable{[](const auto &sub) -> ExpectedSystemPosture {
                        if constexpr (requires { sub.expectedSuccess; })
                            return sub.expectedSuccess;
                        return {};
                    }},
                    s.subState);

                // A service error during operations crashes the macro-state down to Fault
                return StateError{.posture = s.posture.invalidate(failedExpectation), .message = e.reason};
            },
            [&](const StateOperational &s, const auto &) -> SystemState {
                // Delegate to Sub-FSM (MasterFSM.transitions_operational.cpp)
                OperationalState nextSubState = this->processOperationalTransition(s, event);

                // Return a fresh copy of the macro state containing the new Level 2 sub-state.
                // Note: The posture itself is not modified here. Postures update on `EvServiceSuccess`.
                return StateOperational{.posture = s.posture, .subState = nextSubState};
            },

            // --- FAULT RECOVERY ---
            [&](const StateError &s, const CmdResetError &) -> SystemState {
                qInfo() << "MFSM: Error reset acknowledged. Forcing reinitialization.";
                // TODO: Should not force re-initialization, find a way to recover properly though (`RecoveryService` ? `IHomingService::try_recover()` ?)
                return StateWaitingInitialization{};
            },

            // Fallback: Event not handled for this Macro State
            [&](const auto &, const auto &) -> SystemState {
                return currentState;
            });

        return std::visit(museum, currentState, event);
    }

    // ==========================================
    // STATIC EVENTS (Pad Movements & Configs)
    // ==========================================
    // These events interact with the machine but DO NOT alter the State Machine indexes.
    bool MasterFSM::processStaticEvent(SystemState &currentState, const SystemEvent &event)
    {
        bool processed = true;

        const auto museum = overloadedCallable(
            // --- CAMERAS PARAMETERS ---
            // Allowed anywhere (even in Faults to debug visually)
            [&](const auto &, const CmdCameraParamUpdate &cmd) {
                emit s_requestCameraParamUpdate(cmd.cameraId, cmd.kind, cmd.value);
            },

            // --- MANUAL PADS (Only allowed during Operational Alignment mode) ---
            [&](StateOperational &opState, const CmdAlignmentPad &cmd) {
                if (auto *alignState = std::get_if<StateAlignment>(&opState.subState))
                {
                    if (alignState->phase == ContactPhase::Free && !alignState->isAutoAlignment)
                    {
                        this->processCmdAlignmentPad(cmd);
                    }
                    else
                    {
                        emit s_warningOccurred("Alignment pad disabled during contact or auto-mode.");
                    }
                }
                else
                {
                    emit s_warningOccurred("Alignment pad only available in Alignment Mode.");
                }
            },
            [&](StateOperational &opState, const CmdVisionPad &cmd) {
                if (auto *alignState = std::get_if<StateAlignment>(&opState.subState))
                {
                    if (!alignState->isAutoAlignment)
                    {
                        this->processCmdVisionPad(cmd);
                    }
                }
                else
                {
                    emit s_warningOccurred("Vision pad only available in Alignment Mode.");
                }
            },
            [&](StateOperational &opState, const CmdZAxisPad &cmd) {
                if (auto *alignState = std::get_if<StateAlignment>(&opState.subState))
                {
                    // Manual Z is strictly prohibited while auto-contact algorithms are moving Z
                    if (alignState->phase == ContactPhase::Free || alignState->phase == ContactPhase::InContact)
                    {
                        this->processCmdZPad(cmd);
                    }
                    else
                    {
                        emit s_warningOccurred("Z-Axis pad locked. Automated routine in progress.");
                    }
                }
                else
                {
                    emit s_warningOccurred("Z-Axis pad only available in Alignment Mode.");
                }
            },
            [&](StateOperational &opState, const CmdApplyContact &cmd) {
                if (auto *alignState = std::get_if<StateAlignment>(&opState.subState))
                {
                    auto guard = Interlocks::canApplyContact(opState.posture);
                    if (!guard)
                    {
                        emit s_warningOccurred(QString::fromUtf8(guard.unwrap_err()));
                        return;
                    }
                    alignState->phase = ContactPhase::ApplyingContact;
                    m_alignmentService->setHardwareLock(true);
                    m_contactService->startContactRoutine(Services::BasicContactPayload{.forceGF = cmd.forceGF});
                }
            },

            // Fallback (Not a static event)
            [&](const auto &, const auto &) { processed = false; });

        std::visit<void>(museum, currentState, event);
        return processed;
    }

} // namespace Kub3::MFSM
