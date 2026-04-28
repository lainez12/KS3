#include <QDebug>

#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <MFSM/MasterFSM.h>
#include <Services/Contact/ContactService.h>

namespace
{
    struct BootDependency {
        std::string subsystemId;
        std::string readyKey;
    };
}

namespace Kub3::MFSM
{

    MasterFSM::MasterFSM(Shared<HAL::MS::IMachineStatusRepo> repo,
                         Shared<Services::IHomingService> homingService,
                         Shared<Services::IDrawerService> drawerService,
                         Shared<Services::IAlignmentService> alignmentService,
                         Shared<Services::IVisionService> visionService,
                         Shared<Services::IContactService> contactService,
                         QObject *parent) :
        QObject(parent),
        m_state(StateBooting{}),
        m_repo(std::move(repo)),
        m_homingService(std::move(homingService)),
        m_drawerService(std::move(drawerService)),
        m_alignmentService(std::move(alignmentService)),
        m_visionService(std::move(visionService)),
        m_contactService(std::move(contactService)),
        m_logicTimer(this)
    {
        connect(&m_logicTimer, &QTimer::timeout, this, &MasterFSM::onLogicTick);
    }

    void MasterFSM::start(void)
    {
        m_logicTimer.start(20);         // 50Hz
        emit s_stateChanged("BOOTING"); // Signal the initial state to the UI
    }

    // ==========================================
    // HEARTBEAT LOGIC
    // ==========================================

    void MasterFSM::onLogicTick(void)
    {
        checkHardwareSafety(); // Unconditional Safety Check (e.g. Physical Emergency-Stop Button)

        const auto museum = overloadedCallable(
            [&](StateBooting &bootState) { onStateBootingTick(bootState); },
            [&](StateWaitingInitialization &) { /* Wait for trigger */ },
            [&](StateInitialization &initState) { onStateInitializationTick(initState); },
            [&](StateIdle &) { /* Monitor temperatures, hold position */ },
            [&](StateOperating &operatingState) { onStateOperatingTick(operatingState); },
            [&](StateError &) { /* Blink red lights */ },
            [&](StateEmergencyStop &) { /* Ensure actuators are disabled */ },
            [&](StatePowerOff &powerOffState) { onStatePowerOffTick(powerOffState); });

        std::visit(museum, m_state); // Execute State-Specific Continuous Logic
    }

    void MasterFSM::onStateBootingTick(StateBooting &bootState)
    {
        bootState.ticksElapsed++;

        // (In the future, this list could be injected via ApplicationBuilder from config)
        static const std::vector<BootDependency> requiredDependencies = {
#if defined(KUB_MODEL_8)
            {MCU_ARDUINO1_ID, MCU_ARDUINO1_READY},
            {MCU_ARDUINO2_ID, MCU_ARDUINO2_READY},
            {MCU_ARDUINO3_ID, MCU_ARDUINO3_READY}
#endif
        };

        constexpr uint32_t TICKS_PER_SEC = 50;
        constexpr uint32_t TIMEOUT_TICKS = 5 * TICKS_PER_SEC; // 5 seconds
        constexpr uint8_t MAX_RETRIES    = 3;

        bool allReady             = true;
        const bool timeoutReached = (bootState.ticksElapsed >= TIMEOUT_TICKS);
        bool fatalFailure         = false;
        std::vector<std::string> missingDependencies;

        for (const auto &dep : requiredDependencies)
        {
            if (!HAL::MS::readBool(m_repo, dep.readyKey))
            {
                allReady = false;
                missingDependencies.push_back(dep.subsystemId);

                // Timeout & Retry Logic
                if (timeoutReached)
                {
                    if (bootState.retryCounts[dep.subsystemId] < MAX_RETRIES)
                    {
                        bootState.retryCounts[dep.subsystemId]++;
                        qWarning() << std::format("MFSM: Timeout on {}. Requesting targeted retry ({}/{})", dep.subsystemId, bootState.retryCounts[dep.subsystemId], MAX_RETRIES);
                        emit s_requestHardwareRetry(QString::fromStdString(dep.subsystemId)); // Route the restart request for this specific subsystem
                    }
                    else
                    {
                        fatalFailure = true;
                    }
                }
            }
        }

        if (fatalFailure)
        {
            // Joining error strings with ", " separator
            std::string errorMsg = "Hardware timeout on: " + missingDependencies.front();

            errorMsg = std::accumulate(
                std::next(missingDependencies.begin()),
                missingDependencies.end(),
                errorMsg,
                [](const std::string &a, const std::string &b) { return std::move(a) + ", " + std::move(b); });

            qCritical() << std::format("MFSM: Max retries reached. {}", errorMsg).c_str();
            dispatch(EvHardwareError{errorMsg});
            return;
        }

        if (allReady)
            dispatch(EvHardwareReady{});
        else if (timeoutReached)        // Timeout & not all ready
            bootState.ticksElapsed = 0; // Reset timer for the next retry window
    }

    void MasterFSM::onStateInitializationTick(StateInitialization &state)
    {
        m_homingService->tick();

        const Services::ServiceStatus status = m_homingService->getStatus();

        if (status == Services::ServiceStatus::Success)
            dispatch(EvInitializationComplete{});
        else if (status == Services::ServiceStatus::Error)
            dispatch(EvServiceError{.reason = m_homingService->getErrorReason()});
    }

    void MasterFSM::onStateOperatingTick(StateOperating &op)
    {
        const auto museum = overloadedCallable{
            [&](const Payloads::DrawerOpPayload &) { basicOperatingServiceTick(m_drawerService.get()); },
            [&](const Payloads::HomingOpPayload &) { basicOperatingServiceTick(m_homingService.get()); },
            [&](const Payloads::AlignmentOpPayload &payload) {
                m_alignmentService->tick();
                m_visionService->tick();
                m_contactService->tick();

                if (payload.phase == Payloads::AlignmentPhase::ApplyingContact ||
                    payload.phase == Payloads::AlignmentPhase::Separating)
                {
                    const Services::ServiceStatus zStatus = m_contactService->getStatus();

                    if (zStatus == Services::ServiceStatus::Success)
                        dispatch(EvContactSequenceComplete{});
                    else if (zStatus == Services::ServiceStatus::Error)
                        dispatch(EvServiceError{.reason = m_contactService->getErrorReason()});
                }
            },
        };

        std::visit(museum, op.payload);
    }

    void MasterFSM::basicOperatingServiceTick(Services::IService *service)
    {
        if (!service)
            return;
        service->tick();

        const Services::ServiceStatus status = service->getStatus();

        if (status == Services::ServiceStatus::Success)
            dispatch(EvServiceSuccess{});
        else if (status == Services::ServiceStatus::Error)
            dispatch(EvServiceError{.reason = service->getErrorReason()});
    }

    void MasterFSM::onStatePowerOffTick(StatePowerOff &powerOffState)
    {
        // TODO: Check homing state before powering off

        emit s_requestPowerOff();
    }

    void MasterFSM::checkHardwareSafety(void)
    {
        const bool emergencyStopTriggered  = HAL::MS::readBool(m_repo, EMERGENCY_STOP_BUTTON);
        const bool systemPowerOffTriggered = HAL::MS::readBool(m_repo, POWER_OFF_BUTTON);

        // Dispatch emergency stop event when:
        //  - Emergency stop press is detected
        //  - Not already in emergency stop state
        if (emergencyStopTriggered && !std::holds_alternative<StateEmergencyStop>(m_state))
        {
            dispatch(EvEmergencyStopTriggered{"Hardware Emergency Stop Button Pressed"});
        }
        if (systemPowerOffTriggered && !std::holds_alternative<StatePowerOff>(m_state))
        {
            dispatch(EvPowerOff{});
        }
    }

    // ==========================================
    // FSM TRANSITION DISPATCHER
    // ==========================================

    void MasterFSM::dispatch(const SystemEvent &event)
    {
        if (processStaticEvent(m_state, event))
            return;

        // Calculate the next state based on current state + event
        SystemState nextState = processTransition(m_state, event);

        // If the state changed, apply it and trigger entry actions
        if (nextState.index() != m_state.index())
        {
            m_state = nextState;
            onStateEntered(m_state);
        }
    }

    // A static event is an event not changing the FSM state
    bool MasterFSM::processStaticEvent(const SystemState &currentState, const SystemEvent &event)
    {
        bool processed    = true; // Whether the event has been processed by the function
        const auto museum = overloadedCallable(
            // Camera parameter command
            [&](const StateIdle &, const CmdCameraParamUpdate &cmd) { emit s_requestCameraParamUpdate(cmd.cameraId, cmd.kind, cmd.value); },
            [&](const StateOperating &, const CmdCameraParamUpdate &cmd) { emit s_requestCameraParamUpdate(cmd.cameraId, cmd.kind, cmd.value); },
#if defined(BUILD_DEBUG)
            [&](const auto &, const CmdCameraParamUpdate &cmd) { emit s_requestCameraParamUpdate(cmd.cameraId, cmd.kind, cmd.value); },
#endif
            // --- ALIGNMENT PAD ---
            [&](const StateOperating &s, const CmdAlignmentPad &cmd) {
                if (const auto *alignPayload = std::get_if<Payloads::AlignmentOpPayload>(&s.payload))
                {
                    if (alignPayload->phase != Payloads::AlignmentPhase::Free)
                    {
                        qWarning() << "MFSM: Alignment movement rejected. Incompatible alignment phase:" << static_cast<int>(alignPayload->phase);
                        return;
                    }
                    if (!alignPayload->isAutoAlignment) // Only accept manual pad commands in manual mode
                        this->processCmdAlignmentPad(cmd);
                }
                else
                    qWarning() << "MFSM: Alignment movement rejected. Not in Alignment mode.";
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
                    qWarning() << "MFSM: Vision movement rejected. Not in Alignment mode.";
            },
            // --- Z PAD ---
            [&](const StateOperating &s, const CmdZAxisPad &cmd) {
                if (const auto *p = std::get_if<Payloads::AlignmentOpPayload>(&s.payload))
                {
                    // Pad-controlled Z movements are only allowed in Free or InContact
                    if (p->phase == Payloads::AlignmentPhase::ApplyingContact ||
                        p->phase == Payloads::AlignmentPhase::Separating)
                    {
                        qWarning() << "MFSM: Z movement rejected. Z is currently automated.";
                        return;
                    }
                    processCmdZPad(cmd); // Routes to ContactService
                }
            },
            // Fallback: event not handled
            [&](const auto &, const auto &) { processed = false; });

        std::visit(museum, currentState, event);
        return processed;
    }

    // Mathematically pure function mapping: (State x Event) -> State
    SystemState MasterFSM::processTransition(const SystemState &currentState, const SystemEvent &event)
    {
        const auto museum = overloadedCallable(
            // Global Event Overrides (Can happen in almost any state)
            [&](const auto &, const EvEmergencyStopTriggered &e) -> SystemState { return StateEmergencyStop{e.reason}; },
            [&](const auto &, const EvPowerOff &e) -> SystemState { return StatePowerOff{}; },
            // Booting -> Waiting for initialization
            [&](const StateBooting &, const EvHardwareReady &) -> SystemState { return StateWaitingInitialization{}; },
            // Booting -> Error
            [&](const StateBooting &, const EvHardwareError &e) -> SystemState { return StateError{e.reason}; },
            // Waiting for initialization -> Initialization
            [&](const StateWaitingInitialization &, const CmdStartInitialization &) -> SystemState { return StateInitialization{}; },
            // Initialization -> Idle
            [&](const StateInitialization &, const EvInitializationComplete &) -> SystemState { return StateIdle{}; },
            // Idle -> Operating Drawer (Insert/Eject)
            [&](const StateIdle &, const CmdOperateDrawer &cmd) -> SystemState {
                const Payloads::DrawerOpPayload payload{
                    .kind   = cmd.operation,
                    .target = cmd.target,
                };
                return StateOperating{payload};
            },
            // Idle -> Operating (Alignment)
            [&](const StateIdle &, const CmdEnterAlignmentMode &cmd) -> SystemState {
                return StateOperating{Payloads::AlignmentOpPayload{.isAutoAlignment = cmd.autoMode}};
            },
            // Operating (Alignment) -> Idle (Only if alignment is manually stopped/finished)
            [&](const StateOperating &s, const CmdExitAlignmentMode &) -> SystemState {
                if (std::holds_alternative<Payloads::AlignmentOpPayload>(s.payload))
                    return StateIdle{};
                return currentState; // Reject if we are homing or moving drawers
            },
            // Operating (Alignment) -> Start running contact routine
            [&](StateOperating &s, const CmdApplyContact &cmd) -> SystemState {
                if (auto *p = std::get_if<Payloads::AlignmentOpPayload>(&s.payload)) // Ignore if not in alignment mode
                {
                    m_alignmentService->setHardwareLock(true);                                                    // Force halt alignment and lock them
                    m_contactService->startContactRoutine(Services::BasicContactPayload{.forceGF = cmd.forceGF}); // Start the ContactService sequence
                    p->phase = Payloads::AlignmentPhase::ApplyingContact;                                         // Update alignment phase
                }
                return s; // Leave state unchanged (maybe I should handle all "static" events like this)
            },
            // Operating (Alignment) -> Handle contact routine finished
            [&](StateOperating &s, const EvContactSequenceComplete &) -> SystemState {
                if (auto *p = std::get_if<Payloads::AlignmentOpPayload>(&s.payload))
                {
                    if (p->phase == Payloads::AlignmentPhase::ApplyingContact)
                        p->phase = Payloads::AlignmentPhase::InContact; // Safely locked in contact
                    else if (p->phase == Payloads::AlignmentPhase::Separating)
                    {
                        p->phase = Payloads::AlignmentPhase::Free;
                        m_alignmentService->setHardwareLock(false); // Unlock alignment
                    }
                }
                return s; // Leave state unchanged
            },
            // Operating -> Idle
            [&](const StateOperating &, const EvServiceSuccess &c) -> SystemState { return StateIdle{}; },
            // Operating -> Error
            [&](const StateOperating &, const EvServiceError &e) -> SystemState { return StateError{e.reason}; },
            // Error -> Idle (Reset)
            [&](const StateError &, const CmdResetError &) -> SystemState { return StateIdle{}; },
            // Fallback: If an event is not handled for the current state, remain in current state.
            [&](const auto &, const auto &) -> SystemState {
                qWarning() << "MFSM: Ignored Event in current state.";
                return currentState;
            });

        return std::visit(museum, currentState, event);
    }

    // Actions executed exactly ONCE upon entering a state
    void MasterFSM::onStateEntered(const SystemState &newState)
    {
        // We ALWAYS send the updated state to the UI so it can update its visual state machine
        const auto nameExtractor = overloadedCallable{
            [](const StateBooting &) { return QString("BOOTING"); },
            [](const StateWaitingInitialization &) { return QString("WAITING_INITIALIZATION"); },
            [](const StateInitialization &) { return QString("INITIALIZING"); },
            [](const StateIdle &) { return QString("IDLE"); },
            [](const StateOperating &) { return QString("OPERATING"); },
            [](const StateError &) { return QString("ERROR"); },
            [](const StateEmergencyStop &) { return QString("EMERGENCY_STOP"); },
            [](const StatePowerOff &) { return QString("POWER_OFF"); },
        };

        emit s_stateChanged(std::visit(nameExtractor, newState));

        // 2. Perform Physical State Entry Actions
        const auto entryActions = overloadedCallable{
            [&](const StateBooting &) { /* no-op */ },
            [&](const StateWaitingInitialization &) { /* no-op */ },
            [&](const StateInitialization &) { m_homingService->initialize(); },
            [&](const StateIdle &) { stopAllServices(); },
            [&](const StateOperating &s) {
                std::visit(
                    overloadedCallable{
                        [&](const Payloads::DrawerOpPayload &payload) {
                            if (payload.kind == DrawerOperation::EJECT)
                                m_drawerService->eject(payload.target);
                            else if (payload.kind == DrawerOperation::INSERT)
                                m_drawerService->insert(payload.target);
                        },
                        [&](const Payloads::HomingOpPayload &payload) { m_homingService->home(payload.target); },
                        [&](const Payloads::AlignmentOpPayload &p) {
                            m_alignmentService->setHardwareLock(p.phase != Payloads::AlignmentPhase::Free);
                        }},
                    s.payload);
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

        std::visit(entryActions, newState);
    }

    void MasterFSM::stopAllServices(void)
    {
        m_homingService->stop();
        m_drawerService->stop();
        m_alignmentService->stop();
        m_contactService->stop();
        m_visionService->stop();
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

    // ==========================================
    // UI SLOTS (User Inputs)
    // ==========================================

    void MasterFSM::ps_requestInitialization(void)
    {
        dispatch(CmdStartInitialization{});
    }

    void MasterFSM::ps_requestOperateDrawer(int targetInt, int operationInt)
    {
        auto target = static_cast<Services::DrawerTarget>(targetInt);
        auto op     = static_cast<DrawerOperation>(operationInt);

        dispatch(CmdOperateDrawer{.target = target, .operation = op});
    }

    void MasterFSM::ps_requestResetError(void)
    {
        dispatch(CmdResetError{});
    }

    void MasterFSM::ps_requestEmergencyStop(void)
    {
        dispatch(EvEmergencyStopTriggered{"Software E-Stop Triggered by Operator"});
    }

    void MasterFSM::ps_systemPowerOff(void)
    {
        dispatch(EvPowerOff{});
    }

    void MasterFSM::ps_requestExposureUpdate(const QString &camId, double exposureRatio)
    {
        dispatch(CmdCameraParamUpdate{
            .cameraId = camId,
            .kind     = HAL::Vision::CameraParamKind::EXPOSURE,
            .value    = exposureRatio,
        });
    }

    void MasterFSM::ps_requestGainUpdate(const QString &camId, double gainRatio)
    {
        dispatch(CmdCameraParamUpdate{
            .cameraId = camId,
            .kind     = HAL::Vision::CameraParamKind::GAIN,
            .value    = gainRatio,
        });
    }

    void MasterFSM::ps_requestFrameRateUpdate(const QString &camId, double framerate)
    {
        dispatch(CmdCameraParamUpdate{
            .cameraId = camId,
            .kind     = HAL::Vision::CameraParamKind::FRAMERATE,
            .value    = framerate,
        });
    }

    void MasterFSM::ps_requestCenteredZoomUpdate(const QString &camId, double zoomFactor)
    {
        dispatch(CmdCameraParamUpdate{
            .cameraId = camId,
            .kind     = HAL::Vision::CameraParamKind::CENTERED_ZOOM,
            .value    = zoomFactor,
        });
    }

    void MasterFSM::ps_requestROIUpdate(const QString &camId, const QRect &roi)
    {
        dispatch(CmdCameraParamUpdate{
            .cameraId = camId,
            .kind     = HAL::Vision::CameraParamKind::REGION_OF_INTEREST,
            .value    = roi,
        });
    }

} // namespace Kub3::MFSM
