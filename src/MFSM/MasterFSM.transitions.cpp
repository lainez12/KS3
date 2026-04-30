#include <MFSM/MasterFSM.h>
#include <Services/Contact/ContactService.h>

namespace Kub3::MFSM
{

    // A static event is an event not changing the FSM state
    bool MasterFSM::processStaticEvent(SystemState &currentState, const SystemEvent &event)
    {
        bool processed = true; // Whether the event has been processed by the function
        auto museum    = overloadedCallable(
            // --- CAMERAS PARAMETERS COMMANDS ---
            [&](const StateIdle &, const CmdCameraParamUpdate &cmd) { emit s_requestCameraParamUpdate(cmd.cameraId, cmd.kind, cmd.value); },
            [&](const StateOperating &, const CmdCameraParamUpdate &cmd) { emit s_requestCameraParamUpdate(cmd.cameraId, cmd.kind, cmd.value); },
#if defined(BUILD_DEBUG)
            [&](const auto &, const CmdCameraParamUpdate &cmd) { emit s_requestCameraParamUpdate(cmd.cameraId, cmd.kind, cmd.value); },
#endif

            // --- CONTACT REQUEST ---
            [&](StateOperating &s, const CmdApplyContact &cmd) {
                if (auto *p = std::get_if<Payloads::AlignmentOpPayload>(&s.payload)) // Ignore if not in alignment mode
                {
                    m_alignmentService->setHardwareLock(true);                                                    // Force halt alignment and lock them
                    m_contactService->startContactRoutine(Services::BasicContactPayload{.forceGF = cmd.forceGF}); // Start the ContactService sequence
                    p->phase = Payloads::ContactPhase::ApplyingContact;                                           // Update alignment phase
                }
            },
            // --- CONTACT ROUTINE FINISHED ---
            [&](StateOperating &s, const EvContactSequenceComplete &) {
                if (auto *p = std::get_if<Payloads::AlignmentOpPayload>(&s.payload))
                {
                    if (p->phase == Payloads::ContactPhase::ApplyingContact)
                        p->phase = Payloads::ContactPhase::InContact; // Safely locked in contact
                    else if (p->phase == Payloads::ContactPhase::Separating)
                    {
                        p->phase = Payloads::ContactPhase::Free;
                        m_alignmentService->setHardwareLock(false); // Unlock alignment
                    }
                }
            },

            // --- START EXPOSURE REQUEST ---
            [&](const StateOperating &s, const CmdStartExposure &cmd) {
                if (auto *p = std::get_if<Payloads::AlignmentOpPayload>(&s.payload))
                {
                    // TODO: lock Z movements while exposure in progress
                    if (p->phase == Payloads::ContactPhase::InContact)
                        m_exposureService->startExposure(cmd.payload);
                }
                else
                {
                    qWarning() << "MFSM: Exposure request ignored. Not in a valid exposure position.";
                }
            },

            // --- ALIGNMENT PAD ---
            [&](const StateOperating &s, const CmdAlignmentPad &cmd) {
                if (const auto *alignPayload = std::get_if<Payloads::AlignmentOpPayload>(&s.payload))
                {
                    if (alignPayload->phase != Payloads::ContactPhase::Free)
                    {
                        qWarning() << "MFSM: Alignment movement rejected. Incompatible alignment phase:" << static_cast<int>(alignPayload->phase);
                        return;
                    }
                    if (!alignPayload->isAutoAlignment) // Only accept manual pad commands in manual mode
                        this->processCmdAlignmentPad(cmd);
                }
                else
                {
                    qWarning() << "MFSM: Alignment movement rejected. Not in Alignment mode.";
                }
            },

            // --- VISION PAD ---
            [&](const StateOperating &s, const CmdVisionPad &cmd) {
                if (const auto *alignPayload = std::get_if<Payloads::AlignmentOpPayload>(&s.payload))
                {
                    // Only block vision pad movements if Auto-Alignment algorithm is running.
                    if (!alignPayload->isAutoAlignment)
                        this->processCmdVisionPad(cmd);
                }
                else
                {
                    qWarning() << "MFSM: Vision movement rejected. Not in Alignment mode.";
                }
            },

            // --- Z PAD ---
            [&](const StateOperating &s, const CmdZAxisPad &cmd) {
                if (const auto *p = std::get_if<Payloads::AlignmentOpPayload>(&s.payload))
                {
                    // Pad-controlled Z movements are only allowed in Free or InContact
                    if (p->phase == Payloads::ContactPhase::ApplyingContact ||
                        p->phase == Payloads::ContactPhase::Separating)
                    {
                        qWarning() << "MFSM: Z movement rejected. Z is currently automated.";
                        return;
                    }
                    processCmdZPad(cmd); // Routes to ContactService
                }
            },
            // Fallback: event not handled
            [&](const auto &, const auto &) { processed = false; });

        /**
         * @note Forces <void> return type to prevent mismatches caused by the compiler inferring
         * different return types for const vs. non-const lambda parameter combinations.
         */
        std::visit<void>(museum, currentState, event);
        return processed;
    }

    // Mathematically pure function mapping: (State x Event) -> State
    SystemState MasterFSM::processTransition(const SystemState &currentState, const SystemEvent &event)
    {
        const auto museum = overloadedCallable(
            // Global Event Overrides (Can happen in almost any state)
            // --- Emergency Stop
            [&](const auto &, const EvEmergencyStopTriggered &e) -> SystemState { return StateEmergencyStop{.posture = {}, .reason = e.reason}; },
            // --- Power Off
            [&](const auto &, const EvPowerOff &e) -> SystemState { return StatePowerOff{}; },

            // Booting & Initialization
            // --- Booting -> Waiting for initialization (Boot success)
            [&](const StateBooting &, const EvHardwareReady &) -> SystemState { return StateWaitingInitialization{}; },
            // --- Booting -> Error (Boot failed)
            [&](const StateBooting &, const EvHardwareError &e) -> SystemState { return StateError{.posture = {}, .message = e.reason}; },
            // --- Waiting for initialization -> Initialization
            [&](const StateWaitingInitialization &, const CmdStartInitialization &) -> SystemState { return StateInitialization{}; },
            // --- Initialization -> Idle (Initialization success)
            [&](const StateInitialization &, const EvInitializationComplete &) -> SystemState {
                return StateIdle{.posture = {.wafer = WaferPosture::Homed, .mask = MaskPosture::Homed, .vision = VisionPosture::Homed}};
            },

            // Machine operations
            // --- Operating -> Idle
            [&](const StateOperating &s, const EvServiceSuccess &c) -> SystemState { return StateIdle{s.posture.merge(s.expectedSuccess)}; },
            // --- Operating -> Error (Operation failed)
            [&](const StateOperating &s, const EvServiceError &e) -> SystemState { return StateError{s.posture.invalidate(s.expectedSuccess), e.reason}; },
            // --- Error -> Idle (Reset / Recover from error state)
            [&](const StateError &s, const CmdResetError &) -> SystemState { return StateIdle{s.posture}; },
            // --- Idle -> Operating Drawer (Insert/Eject)
            [&](const StateIdle &s, const CmdOperateDrawer &cmd) -> SystemState {
                const Payloads::DrawerOpPayload payload{.kind = cmd.operation, .target = cmd.target};
                ExpectedSystemPosture expPosture;

                if ((cmd.target == Services::DrawerTarget::Mask || cmd.target == Services::DrawerTarget::Both))
                    expPosture.newMaskPosture = (cmd.operation == DrawerOperation::EJECT) ? MaskPosture::Ejected : MaskPosture::Homed;
                if ((cmd.target == Services::DrawerTarget::Wafer || cmd.target == Services::DrawerTarget::Both))
                    expPosture.newWaferPosture = (cmd.operation == DrawerOperation::EJECT) ? WaferPosture::Ejected : WaferPosture::Homed;
                return StateOperating{.posture = s.posture, .payload = payload, .expectedSuccess = expPosture};
            },
            // --- Idle -> Operating (Stowage)
            [&](const StateIdle &s, const CmdOperateStowage &cmd) -> SystemState {
                ExpectedSystemPosture expPosture;

                if (cmd.target == Services::StowageTarget::MASK)
                    expPosture.newMaskPosture = MaskPosture::Exposure;
                else if (cmd.target == Services::StowageTarget::WAFER)
                    expPosture.newWaferPosture = WaferPosture::AlignmentZone;
                return StateOperating{.posture = s.posture, .expectedSuccess = expPosture};
            },
            // --- Idle -> Operating (Alignment)
            [&](const StateIdle &s, const CmdEnterAlignmentMode &cmd) -> SystemState {
                const Payloads::AlignmentOpPayload payload{.isAutoAlignment = cmd.autoMode};
                return StateOperating{.posture = s.posture, .payload = payload};
            },
            // --- Operating (Alignment) -> Idle (Only if alignment is manually stopped/finished)
            [&](const StateOperating &s, const CmdExitAlignmentMode &) -> SystemState {
                if (std::holds_alternative<Payloads::AlignmentOpPayload>(s.payload))
                    return StateIdle{s.posture};
                return currentState; // Reject if we are homing or moving drawers
            },

            // Fallback: If an event is not handled for the current state, remain in current state.
            [&](const auto &, const auto &) -> SystemState {
                qWarning() << "MFSM: Ignored Event in current state.";
                return currentState;
            });

        return std::visit(museum, currentState, event);
    }

}
