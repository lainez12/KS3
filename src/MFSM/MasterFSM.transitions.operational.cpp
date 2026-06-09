#include <QDebug>

#include <Common/Enums.h>
#include <MFSM/MasterFSM.h>
#include <MFSM/interlocks.h>
#include <Services/Stowage/StowageService.h>
#include <utils.h>

namespace Kub3::MFSM
{

    // ==========================================================================
    // LEVEL 2: OPERATIONAL SUB-FSM DISPATCHER
    // ==========================================================================
    // Mathematically maps: (OperationalState x Event) -> OperationalState
    // Applies strict Interlock Guards before allowing any physical sequence to begin.
    OperationalState MasterFSM::processOperationalTransition(const StateOperational &opState, const SystemEvent &event)
    {
        const auto museum = overloadedCallable(
            // ==========================================
            // TRANSITIONS FROM IDLE
            // ==========================================
            [&](const StateIdle &s, const CmdOperateDrawer &cmd) -> OperationalState {
                const bool isEject = (cmd.operation == DrawerOperation::EJECT);
                auto guard         = Interlocks::canOperateDrawer(opState.posture, cmd.target, isEject);

                if (!guard)
                {
                    emit s_warningOccurred(QString::fromUtf8(guard.unwrap_err()));
                    return s; // Reject transition
                }

                ExpectedSystemPosture expected{};
                if (cmd.target == DrawerTarget::Mask || cmd.target == DrawerTarget::Both)
                    expected.newMaskPosture = isEject ? MaskPosture::Ejected : MaskPosture::Homed;
                if (cmd.target == DrawerTarget::Wafer || cmd.target == DrawerTarget::Both)
                    expected.newWaferPosture = isEject ? WaferPosture::Ejected : WaferPosture::Homed;

                return StateDrawerOp{.kind = cmd.operation, .target = cmd.target, .expectedSuccess = expected};
            },

            [&](const StateIdle &s, const CmdOperateStowage &cmd) -> OperationalState {
                auto guard = Interlocks::canOperateStowage(opState.posture, cmd.target);

                if (!guard)
                {
                    emit s_warningOccurred(QString::fromUtf8(guard.unwrap_err()));
                    return s;
                }

                ExpectedSystemPosture expected{};
                if (cmd.target == Services::StowageTarget::WAFER)
                    expected.newWaferPosture = WaferPosture::AlignmentZone;
                else
                    expected.newMaskPosture = MaskPosture::Exposure;

                return StateStowing{.expectedSuccess = expected};
            },

            [&](const StateIdle &s, const CmdOperateUnstowage &cmd) -> OperationalState {
                auto guard = Interlocks::canOperateUnstowage(opState.posture, cmd.target);
                if (!guard)
                {
                    emit s_warningOccurred(QString::fromUtf8(guard.unwrap_err()));
                    return s;
                }

                // Granularly calculate expected success based on requested homing bits
                ExpectedSystemPosture expected{};

                if (cmd.target & Services::StowageTarget::MASK)
                    expected.newMaskPosture = MaskPosture::Homed;
                if (cmd.target & Services::StowageTarget::WAFER)
                    expected.newWaferPosture = WaferPosture::Homed;

                return StateUnstowing{.target = cmd.target, .expectedSuccess = expected};
            },

            [&](const StateIdle &s, const CmdEnterAlignmentMode &cmd) -> OperationalState {
                auto guard = Interlocks::canEnterAlignment(opState.posture);

                if (!guard)
                {
                    emit s_warningOccurred(QString::fromUtf8(guard.unwrap_err()));
                    return s;
                }
                return StatePreparingAlignment{
                    .alignment       = {.phase = ContactPhase::Free, .isAutoAlignment = cmd.autoMode},
                    .expectedSuccess = {.newVisionPosture = VisionPosture::Visualisation},
                };
            },

            // ==========================================
            // SEQUENCE COMPLETIONS (Returning to Idle)
            // ==========================================

            // @note: The macro tick will update opState.posture based on expectedSuccess before this.
            [&](const StateDrawerOp &, const EvServiceSuccess &) -> OperationalState { return StateIdle{}; },
            [&](const StateStowing &, const EvServiceSuccess &) -> OperationalState { return StateIdle{}; },

            // Alingment -> Idle
            [&](const StateAlignment &s, const CmdExitAlignmentMode &) -> OperationalState {
                if (s.phase != ContactPhase::Free)
                {
                    emit s_warningOccurred("Cannot exit alignment mode while Wafer is not free from contact with Mask.");
                    return s;
                }
                // return StatePreparingAlignmentExit{};
                return StateIdle{};
            },

            // ==========================================
            // ALIGNMENT / EXPOSURE LIFECYCLE
            // ==========================================

            // Preparing Alignment -> Alignment (Vision deck is in position)
            [&](const StatePreparingAlignment &s, const EvServiceSuccess &) -> OperationalState {
                return s.alignment; // Vision deck in visualisation position. Ready to align.
            },

            // Contact Sequence Tracker
            [&](StateAlignment s, const EvContactSequenceComplete &) -> OperationalState {
                if (s.phase == ContactPhase::ApplyingContact)
                    s.phase = ContactPhase::InContact;
                else if (s.phase == ContactPhase::Separating)
                    s.phase = ContactPhase::Free;
                return s; // Remain in alignment, but phase is updated
            },

            // Alignment -> Preparing Exposure
            [&](const StateAlignment &s, const CmdEnterExposureMode &cmd) -> OperationalState {
                // Proceed to preparation (moves vision deck out of the way: to home)
                return StatePreparingExposure{
                    .savedContactPhase = s.phase,
                    .expectedSuccess   = {.newVisionPosture = VisionPosture::Homed},
                };
            },

            // Preparing Exposure -> Exposure Ready (Vision deck is homed)
            [&](const StatePreparingExposure &s, const EvServiceSuccess &) -> OperationalState {
                return StateExposureReady{.savedContactPhase = s.savedContactPhase};
            },

            // Exposure Ready -> Exposing
            [&](const StateExposureReady &s, const CmdStartExposure &cmd) -> OperationalState {
                // Check posture requirements before firing UV
                if (auto guard = Interlocks::canStartExposure(opState.posture); !guard)
                {
                    emit s_warningOccurred(QString::fromUtf8(guard.unwrap_err()));
                    return s;
                }
                return StateExposing{.savedContactPhase = s.savedContactPhase, .payload = cmd.payload}; // Start exposing using provided payload.
            },

            // Exposing -> Exposure Ready (UV finished)
            [&](const StateExposing &s, const EvServiceSuccess &) -> OperationalState {
                return StateExposureReady{.savedContactPhase = s.savedContactPhase};
            },

            // Exposure Ready -> Preparing Alignment (Going back)
            [&](const StateExposureReady &s, const CmdEnterAlignmentMode &cmd) -> OperationalState {
                return StatePreparingAlignment{
                    .alignment       = {.phase = s.savedContactPhase, .isAutoAlignment = cmd.autoMode},
                    .expectedSuccess = {.newVisionPosture = VisionPosture::Visualisation},
                };
            },

            // ==========================================
            // FALLBACK
            // ==========================================
            [&](const auto &s, const auto &) -> OperationalState {
                // Event does not trigger a transition in the current sub-state.
                return s;
            });

        return std::visit(museum, opState.subState, event);
    }

} // namespace Kub3::MFSM
