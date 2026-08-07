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
                return StateError{
                    .posture        = {},
                    .severity       = ErrorSeverity::Fatal,
                    .message        = e.reason,
                    .allowedActions = ErrorAction::RetryConnection | ErrorAction::PowerOff,
                };
            },

            // --- INITIALIZATION ---
            [&](const StateWaitingInitialization &, const CmdStartInitialization &) -> SystemState {
                return StateInitializing{};
            },
            [&](const StateInitializing &, const EvInitializationComplete &) -> SystemState {
                const SystemPosture posture = {
                    .wafer  = WaferPosture::Homed,
                    .mask   = MaskPosture::Homed,
                    .vision = VisionPosture::Homed,
                };

                emit s_postureChanged(posture);
                // Init successful: Provide the baseline posture and enter operational `Idle` state
                return StateOperational{
                    .posture  = posture,
                    .subState = StateIdle{},
                };
            },
            [&](const StateInitializing &, const EvServiceError &e) -> SystemState {
                return StateError{
                    .posture        = {},
                    .severity       = ErrorSeverity::Critical,
                    .message        = e.reason,
                    .allowedActions = ErrorAction::ResetMachine | ErrorAction::PowerOff,
                };
            },
            [&](const StateInitializing &, const CmdAbortOperation &) -> SystemState {
                qInfo() << "MFSM: User aborted initialization.";
                this->stopAllServices();
                return StateWaitingInitialization{};
            },

            // --- OPERATIONAL (Delegation to Level 2) ---
            [&](const StateOperational &s, const CmdAbortOperation &e) -> SystemState {
                qInfo() << "MFSM: User aborted sequence. Computing updated physical posture.";

                // Museum dynamically extracting the expected success/abort posture
                const auto museum                                 = overloadedCallable{[](const auto &sub) {
                    ExpectedSystemPosture abort{}, success{};
                    if constexpr (requires { sub.expectedAbort; })
                        abort = sub.expectedAbort;
                    if constexpr (requires { sub.expectedSuccess; })
                        success = sub.expectedSuccess;

                    return std::make_pair(abort, success);
                }};
                SystemPosture newPosture                          = s.posture;
                const auto [abortExpectation, successExpectation] = std::visit(museum, s.subState);

                if (abortExpectation.hasValue())
                {
                    newPosture.merge(abortExpectation);
                }
                else
                {
                    // Fallback: If a state doesn't define expectedAbort but expects a success
                    // (e.g. Vision deck moving), we invalidate the posture making it Unknown.
                    newPosture = newPosture.invalidate(successExpectation);
                }

                emit s_postureChanged(newPosture);
                emit s_operationCanceled();
                // The FSM dispatcher will detect the change to StateIdle and automatically
                // trigger onOperationalStateEntered(StateIdle), which calls stopAllServices().
                return StateOperational{
                    .posture  = newPosture,
                    .subState = StateIdle{}, // Drop cleanly back to Idle
                };
            },

            // @note: Here, using `SystemEvent` has the same effect as using `auto`.
            [&](const StateOperational &s, const auto &event) -> SystemState {
                // 1. Delegate to Sub-FSM (in MasterFSM.transitions.operational.cpp)
                OperationalState nextSubState = this->processOperationalTransition(s, event);

                // 2. Check if the Sub-FSM actually handled the transition.
                if (nextSubState.index() == s.subState.index())
                {
                    // The Sub-FSM ignored the event.
                    // If it was an Error, it means the Sub-FSM didn't know how to recover from it.
                    if constexpr (std::is_same_v<std::decay_t<decltype(event)>, EvServiceError>)
                    {
                        const ExpectedSystemPosture failedExpectation = std::visit(
                            overloadedCallable{[](const auto &sub) -> ExpectedSystemPosture {
                                if constexpr (requires { sub.expectedSuccess; })
                                    return sub.expectedSuccess;
                                return {};
                            }},
                            s.subState);

                        // ESCALATE TO SYSTEM FAULT.
                        return StateError{
                            .posture        = s.posture.invalidate(failedExpectation),
                            .severity       = ErrorSeverity::Critical,
                            .message        = event.reason,
                            .allowedActions = ErrorAction::ResetMachine | ErrorAction::Recover,
                        };
                    }
                }

                // 3. The Sub-FSM successfully handled the event.
                SystemPosture newPosture = s.posture;

                // If it handled an EvServiceError, it means it triggered an automatic recovery.
                // We must invalidate the posture of whatever was moving before recovering.
                if constexpr (std::is_same_v<std::decay_t<decltype(event)>, EvServiceError>)
                {
                    const ExpectedSystemPosture failedExpectation = std::visit(
                        overloadedCallable{[](const auto &sub) -> ExpectedSystemPosture {
                            if constexpr (requires { sub.expectedSuccess; })
                                return sub.expectedSuccess;
                            return {};
                        }},
                        s.subState);

                    newPosture = newPosture.invalidate(failedExpectation);
                }

                // Return a fresh copy of the macro state containing the new Level 2 sub-state.
                return StateOperational{.posture = newPosture, .subState = nextSubState};
            },

            // --- FAULT RECOVERY ---
            [&](const StateError &s, const CmdRetryBoot &) -> SystemState {
                if ((s.allowedActions & ErrorAction::RetryConnection) == ErrorAction::None)
                    return s;
                qInfo() << "MFSM: Retrying MCU connections.";
                return StateBooting{}; // Send FSM back to the very beginning
            },
            [&](const StateError &s, const CmdResetError &) -> SystemState {
                if ((s.allowedActions & ErrorAction::ResetMachine) == ErrorAction::None)
                    return s;
                // TODO: Should not force re-initialization, find a way to recover properly though (`RecoveryService` ? `IHomingService::try_recover()` ?)
                qInfo() << "MFSM: Error reset acknowledged. Forcing reinitialization.";
                return StateWaitingInitialization{};
            },

            // Fallback: Event not handled for this Macro State
            [&](const auto &, const auto &) -> SystemState {
                qDebug() << "No matches found: event skipped.";
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
            [&](StateOperational &opState, const CmdVisualisation &cmd) {
                if (auto *alignState = std::get_if<StateAlignment>(&opState.subState))
                {
                    if (!alignState->isAutoAlignment)
                    {
                        this->processCmdVisualisation(cmd);
                    }
                }
                else
                {
                    static const char *warningMsg = "Vision commands are only available in Alignment Mode.";

                    qWarning() << warningMsg;
                    emit s_warningOccurred(warningMsg);
                }
            },
            [&](StateOperational &opState, const CmdZAxisPad &cmd) {
                if (auto *alignState = std::get_if<StateAlignment>(&opState.subState))
                {
                    // Safety Guard: Forbid manual Z movement if Air is active
                    if (std::holds_alternative<Services::ZMovePayload>(cmd.operation) &&
                        m_contactService->isSubstrateCompressedAirActive())
                    {
                        emit s_warningOccurred("Z-Axis pad locked. Compressed air is actively clamping the mask and wafer.");
                        return;
                    }
                    // Operator manually touched the Z-pad. Void the automated air authorization.
                    this->setCompressedAirAuthorized(false);

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
            [&](StateOperational &opState, const CmdSetSubstrateCompressedAir &cmd) {
                if (cmd.enableCompressedAir)
                {
                    auto r = Interlocks::canEnableCompressedAir(opState.posture, opState);

                    if (!r)
                    {
                        qWarning() << r.unwrap_err();
                        emit s_warningOccurred(r.unwrap_err());
                        return;
                    }
                    if (!m_compressedAirAuthorized)
                    {
                        static const char *err = "Action Rejected - Compressed Air requires a successful automated contact sequence. "
                                                 "Manual adjustments void this authorization.";

                        qWarning() << err;
                        emit s_warningOccurred(err);
                        return;
                    }
                }
                m_contactService->setSubstrateCompressedAir(cmd.enableCompressedAir);
            },

            // Fallback (Not a static event)
            [&](const auto &, const auto &) { processed = false; });

        std::visit<void>(museum, currentState, event);
        return processed;
    }

} // namespace Kub3::MFSM
