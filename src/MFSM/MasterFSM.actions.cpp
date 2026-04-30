#include <MFSM/MasterFSM.h>

namespace Kub3::MFSM
{

    // Actions executed exactly ONCE upon entering a state
    void MasterFSM::onStateEntered(const SystemState &newState)
    {
        // We ALWAYS send the updated state to the UI so it can update its visual state machine
        const auto nameExtractorMuseum = overloadedCallable{
            [](const StateBooting &) { return QString("BOOTING"); },
            [](const StateWaitingInitialization &) { return QString("WAITING_INITIALIZATION"); },
            [](const StateInitialization &) { return QString("INITIALIZING"); },
            [](const StateIdle &) { return QString("IDLE"); },
            [](const StateOperating &) { return QString("OPERATING"); },
            [](const StateError &) { return QString("ERROR"); },
            [](const StateEmergencyStop &) { return QString("EMERGENCY_STOP"); },
            [](const StatePowerOff &) { return QString("POWER_OFF"); },
        };

        emit s_stateChanged(std::visit(nameExtractorMuseum, newState));

        // 2. Perform Physical State Entry Actions
        const auto entryActionsMuseum = overloadedCallable{
            [&](const StateBooting &) { /* no-op */ },
            [&](const StateWaitingInitialization &) { /* no-op */ },
            [&](const StateInitialization &) { m_homingService->initialize(); },
            [&](const StateIdle &) { stopAllServices(); },
            [&](const StateOperating &s) {
                const auto operatingMuseum = overloadedCallable{
                    [&](const Payloads::HomingOpPayload &payload) { m_homingService->home(payload.target); },
                    [&](const Payloads::DrawerOpPayload &payload) {
                        if (payload.kind == DrawerOperation::EJECT)
                            m_drawerService->eject(payload.target);
                        else if (payload.kind == DrawerOperation::INSERT)
                            m_drawerService->insert(payload.target);
                    },
                    [&](const Payloads::StowageOpPayload &p) { m_stowageService->loadWaferToAlignment(); },
                    [&](const Payloads::AlignmentOpPayload &p) {
                        m_alignmentService->setHardwareLock(p.phase != Payloads::ContactPhase::Free);
                    },
                    [&](const Payloads::ExposureOpPayload &p) { m_exposureService->startExposure(p.payload); }};

                std::visit(operatingMuseum, s.payload);
            },
            [&](const StateError &s) {
                stopAllServices();
                emit s_errorOccurred(QString::fromStdString(s.message));
            },
            [&](const StateEmergencyStop &s) {
                stopAllServices();
                emit s_errorOccurred(QString::fromStdString(s.reason));
            },
            [&](const StatePowerOff &s) {
                stopAllServices();
                // TODO: initiate homing if possible and wait for it to end before poweroff
            }};

        std::visit(entryActionsMuseum, newState);
    }

    void MasterFSM::stopAllServices(void)
    {
        m_homingService->stop();
        m_drawerService->stop();
        m_stowageService->stop();
        m_alignmentService->stop();
        m_visionService->stop();
        m_contactService->stop();
        m_exposureService->stop();
    }

    void MasterFSM::processCmdAlignmentPad(const CmdAlignmentPad &cmd)
    {
        const auto museum = overloadedCallable{
            [&](const Services::AlignmentMoveStagePayload &p) {
                m_alignmentService->moveStage(cmd.targetStage, p.dir);
            },
            [&](const Services::AlignmentStopStagePayload &) {
                m_alignmentService->stopStage(cmd.targetStage);
            },
            [&](const Services::AlignmentSetKinematicModePayload &p) {
                m_alignmentService->setKinematicProfile(cmd.targetStage, p.fineMode);
            }};

        std::visit(museum, cmd.operation);
    }

    void MasterFSM::processCmdVisionPad(const CmdVisionPad &cmd)
    {
        const auto museum = overloadedCallable{
            [&](const Services::VisionMovePayload &p) {
                m_visionService->moveManual(cmd.targetMotor, p.dir);
            },
            [&](const Services::VisionStopPayload &) {
                m_visionService->stopManual(cmd.targetMotor);
            },
            [&](const Services::VisionSetKinematicModePayload &p) {
                m_visionService->setKinematicMode(cmd.targetMotor, p.fineMode);
            },
            [&](const Services::VisionSetPushingModePayload &p) {
                m_visionService->setPushingMode(p.enable);
            }};

        std::visit(museum, cmd.operation);
    }

    void MasterFSM::processCmdZPad(const CmdZAxisPad &cmd)
    {
        if (auto *payload = std::get_if<Services::ZMovePayload>(&cmd.operation))
            m_contactService->moveZManual(payload->direction);
        else if (auto *p = std::get_if<Services::ZStopPayload>(&cmd.operation))
            m_contactService->stopZManual();
    }

}
