#include <Common/Enums.h>
#include <MFSM/MasterFSM.h>
#include <Services/Contact/ContactService.h>
#include <Services/Stowage/StowageService.h>
#include <utils.h>

namespace Kub3::MFSM
{

    // ==========================================================================
    // MACRO-STATE ENTRY ACTIONS
    // ==========================================================================
    void MasterFSM::onStateEntered(const SystemState &newState)
    {
        // Broadcast entered state
        emit s_systemStateKindChanged(kindOf(newState));

        // Perform Physical Macro State Entry Actions
        const auto entryActionsMuseum = overloadedCallable{
            [&](const StateBooting &) { /* no-op */ },
            [&](const StateWaitingInitialization &) { /* no-op */ },
            [&](const StateInitializing &) { m_homingService->initialize(); },
            // Chain the entry action down into the sub-FSM so the initial sub-state (Idle) fires
            [&](const StateOperational &s) { this->onOperationalSubstateEntered(s, s.subState); },
            [&](const StateError &s) {
                this->stopAllServices(); // Non-blocking Services stop
                emit s_errorOccurred(ErrorPayload{
                    .kind           = ErrorKind::Global,
                    .severity       = s.severity,
                    .message        = QString::fromStdString(s.message),
                    .allowedActions = s.allowedActions,
                });
            },
            [&](const StateEmergencyStop &s) {
                qCritical() << "MFSM: EMERGENCY STOP ENTERED - Stopping all hardware.";
                this->stopAllServices();
                emit s_errorOccurred(ErrorPayload{
                    .kind           = ErrorKind::Global,
                    .severity       = ErrorSeverity::Fatal,
                    .message        = QStringLiteral("EMERGENCY-STOP: ") + QString::fromStdString(s.reason),
                    .allowedActions = ErrorAction::ResetMachine | ErrorAction::PowerOff,
                });
            },
            [&](const StatePreparePowerOff &s) {
                qWarning() << "MFSM: Preparing machine power off.";
                this->stopAllServices();
                m_homingService->home(); // Initiate homing before triggering poweroff sequence
            },
            [&](const StatePowerOff &s) {
                qWarning() << "MFSM: Power off sequence initiated.";
                emit s_requestPowerOff(); // Trigger poweroff sequence
            }};

        std::visit(entryActionsMuseum, newState);
    }

    // ==========================================================================
    // OPERATIONAL SUB-STATE ENTRY ACTIONS
    // ==========================================================================
    void MasterFSM::onOperationalSubstateEntered(const StateOperational &parentState, const OperationalState &newSubState)
    {
        using HT = Services::HomingTarget::Type;

        // Broadcast operational state
        emit s_operationalSubstateChanged(newSubState);
        emit s_operationalSubstateKindChanged(kindOf(newSubState));
        qInfo() << "[MasterFSM::onOperationalSubstateEntered]" << toString(newSubState);

        // Reset compressed air toggle authorization cleanly on any micro-state change
        this->setCompressedAirAuthorized(false);

        // Trigger Physical Micro Side-Effects
        const auto entryActionsMuseum = overloadedCallable{
            // Ensure all operational hardware locks are released
            [&](const StateIdle &) {
                this->stopAllServices();
                m_alignmentService->setHardwareLock(false);
            },
            [&](const StateDrawerOp &s) {
                if (s.kind == DrawerOperation::EJECT)
                    m_drawerService->eject(s.target);
                else
                    m_drawerService->insert(s.target);
            },
            [&](const StateStowing &s) { m_stowageService->startStowage(s.target); },
            [&](const StateUnstowing &s) {
                HT target = static_cast<HT>(
                    (has_flag(s.target, StowageTarget::Wafer) ? (HT::ALIGNMENT_STAGES | HT::Z_MOTORS) : 0x0) |
                    (has_flag(s.target, StowageTarget::Mask) ? HT::MASK_CONVEYOR : 0x0));

                m_homingService->home(target);
            },
            [&](const StateAutoleveling &s) {
                m_contactService->startContactRoutine(Services::AutolevelingPayload{});
            },
            [&](const StateRetractingZ &s) {
                m_contactService->retractFromContact();
            },
            [&](const StatePreparingAlignment &s) {
                emit s_processMessageBroadcast({"Preparing for Alignment - Positioning Vision Block."});
                // Move vision deck above the substrate in order to be able to observe it.
                m_visionService->moveBlockToVisualisationPosition();
            },
            [&](const StateAlignment &s) {
                // If we enter alignment and are not contact-free: lock the hardware
                m_alignmentService->setHardwareLock((s.phase != ContactPhase::Free || m_contactService->isInContact()));
            },
            [&](const StatePreparingExposure &) {
                emit s_processMessageBroadcast({"Preparing for Exposure: Moving Vision Block to Home."});
                // Vision deck is in the led lights path. Home it safely before firing UV.
                m_homingService->home(static_cast<HT>(HT::DECK | HT::CAMERAS));
            },
            [&](const StateExposureReady &) {
                emit s_processMessageBroadcast({QString("Vision is clear. Ready to fire UV Exposure.")});
            },
            [&](const StateExposing &s) {
                emit s_processMessageBroadcast({QString("Firing UV Exposure.")});
                m_exposureService->startExposure(s.payload);
            }};

        std::visit<void>(entryActionsMuseum, newSubState);
    }

} // namespace Kub3::MFSM