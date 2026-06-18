#include <QDebug>

#include <Common/Enums.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <MFSM/MasterFSM.h>

namespace Kub3::MFSM
{

    // ==========================================================================
    // LIFECYCLE & INJECTION
    // ==========================================================================
    MasterFSM::MasterFSM(Shared<HAL::MS::IMachineStatusRepo> repo,
                         Shared<Services::IHomingService> homingService,
                         Shared<Services::IDrawerService> drawerService,
                         Shared<Services::IStowageService> stowageService,
                         Shared<Services::IAlignmentService> alignmentService,
                         Shared<Services::IVisionService> visionService,
                         Shared<Services::IContactService> contactService,
                         Shared<Services::IExposureService> exposureService,
                         QObject *parent) :
        QObject(parent),
        m_state(StateBooting{}), // Deterministic initial state,
        m_repo(std::move(repo)),
        m_homingService(std::move(homingService)),
        m_drawerService(std::move(drawerService)),
        m_stowageService(std::move(stowageService)),
        m_alignmentService(std::move(alignmentService)),
        m_visionService(std::move(visionService)),
        m_contactService(std::move(contactService)),
        m_exposureService(std::move(exposureService)),
        m_logicTimer(this)
    {
        // Hard-wire the FSM Logic Loop to 50Hz (20ms)
        // Zero Thread Blocking: The logic loop must never sleep/wait.
        m_logicTimer.setInterval(20);
        m_logicTimer.setTimerType(Qt::PreciseTimer);
        connect(&m_logicTimer, &QTimer::timeout, this, &MasterFSM::onLogicTick);
    }

    void MasterFSM::start(void)
    {
        qInfo() << "MFSM: Engine starting. Firing initial state hooks.";
        onStateEntered(m_state);
        m_logicTimer.start();
    }

    // ==========================================================================
    // THE CORE DISPATCHER (The Brain)
    // ==========================================================================

    void MasterFSM::dispatch(const SystemEvent &event)
    {
        // 1. Static Events: Do not change state topology (e.g., Pads, Parameters).
        // If it was purely a static command, exit early.
        if (processStaticEvent(m_state, event))
            return;

        // 2. Evaluate physical laws and logical transitions.
        SystemState nextState = processMacroTransition(m_state, event); // Invokes Interlocks under the hood for Operational commands.

        // 3. Topology Detection: Did the state actually change ?
        const bool macroChanged = (m_state.index() != nextState.index());
        bool microChanged       = false;

        if (!macroChanged && std::holds_alternative<StateOperational>(m_state))
        {
            const auto &currentOp = std::get<StateOperational>(m_state);
            const auto &nextOp    = std::get<StateOperational>(nextState);
            // If we are still in Operational, did the sub-task change ?
            microChanged = (currentOp.subState.index() != nextOp.subState.index());
        }

        // 4. Apply State and Trigger Side Effects (Actions)
        if (macroChanged || microChanged)
        {
            m_state = nextState;

            if (macroChanged)
                // A system change (e.g., Booting -> Init, or Operational -> Fault)
                onStateEntered(m_state);
            else if (microChanged)
            {
                // A focused operation change (e.g., Idle -> DrawerOp)
                const auto &opState = std::get<StateOperational>(m_state);
                onOperationalStateEntered(opState, opState.subState);
            }
        }
    }

    // ==========================================================================
    // TIER 1 (UI) -> TIER 2 (LOGIC) PUBLIC SLOTS
    // ==========================================================================
    // These slots securely package primitive UI commands into type-safe SystemEvents.

    void MasterFSM::ps_requestInitialization(void)
    {
        dispatch(CmdStartInitialization{});
    }

    void MasterFSM::ps_requestResetError(void)
    {
        dispatch(CmdResetError{});
    }

    void MasterFSM::ps_requestEmergencyStop(void)
    {
        dispatch(EvEmergencyStopTriggered{"Software Emergency Stop requested"});
    }

    void MasterFSM::ps_systemPowerOff(void)
    {
        dispatch(EvPowerOff{});
    }

    void MasterFSM::ps_requestOperateDrawer(int targetInt, int operationInt)
    {
        dispatch(CmdOperateDrawer{
            .target    = static_cast<DrawerTarget>(targetInt),
            .operation = static_cast<DrawerOperation>(operationInt),
        });
    }

    void MasterFSM::ps_requestStowage(int targetInt)
    {
        dispatch(CmdOperateStowage{
            .target = static_cast<Services::StowageTarget>(targetInt),
        });
    }

    void MasterFSM::ps_requestUnstowage(int targetInt)
    {
        dispatch(CmdOperateUnstowage{
            .target = static_cast<Services::StowageTarget>(targetInt),
        });
    }

    void MasterFSM::ps_requestExposure(const Services::ExposurePayload &payload)
    {
        dispatch(CmdStartExposure{
            .payload = payload,
        });
    }

    // --- Camera Wrappers ---
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
        // Converting QRect to the internal Param payload
        dispatch(CmdCameraParamUpdate{
            .cameraId = camId,
            .kind     = HAL::Vision::CameraParamKind::REGION_OF_INTEREST,
            .value    = roi,
        });
    }

    // ==========================================================================
    // PAD & HARDWARE SERVICE ROUTING
    // ==========================================================================

    void MasterFSM::processCmdAlignmentPad(const CmdAlignmentPad &cmd)
    {
        if (auto *op = std::get_if<Services::AlignmentMoveStagePayload>(&cmd.operation))
            m_alignmentService->moveStage(cmd.targetStage, op->dir);
        else if (auto *op = std::get_if<Services::AlignmentStopStagePayload>(&cmd.operation))
            m_alignmentService->stopStage(cmd.targetStage);
        else if (auto *op = std::get_if<Services::AlignmentSetKinematicModePayload>(&cmd.operation))
            m_alignmentService->setKinematicProfile(cmd.targetStage, op->fineMode);
    }

    void MasterFSM::processCmdZPad(const CmdZAxisPad &cmd)
    {
        if (auto *op = std::get_if<Services::ZMovePayload>(&cmd.operation))
            m_contactService->moveZManual(op->direction);
        else if (auto *op = std::get_if<Services::ZStopPayload>(&cmd.operation))
            m_contactService->stopZManual();
    }

    void MasterFSM::processCmdVisionPad(const CmdVisionPad &cmd)
    {
        if (auto *op = std::get_if<Services::VisionMovePayload>(&cmd.operation))
            m_visionService->moveManual(cmd.targetMotor, op->dir);
        else if (auto *op = std::get_if<Services::VisionStopPayload>(&cmd.operation))
            m_visionService->stopManual(cmd.targetMotor);
        else if (auto *op = std::get_if<Services::VisionSetKinematicModePayload>(&cmd.operation))
            m_visionService->setKinematicMode(cmd.targetMotor, op->fineMode);
        else if (auto *op = std::get_if<Services::VisionSetPushingModePayload>(&cmd.operation))
            m_visionService->setPushingMode(op->enable);
    }

    // ==========================================================================
    // SAFETY & INTERNAL UTILITIES
    // ==========================================================================

    void MasterFSM::checkHardwareSafety(void)
    {
        const bool emergencyStopTriggered = HAL::MS::readBool(m_repo, EMERGENCY_STOP_BUTTON);
        // Dispatch emergency stop event when: emergency stop press is detected AND not already in emergency stop state
        if (emergencyStopTriggered && !std::holds_alternative<StateEmergencyStop>(m_state))
        {
            dispatch(EvEmergencyStopTriggered{"Hardware Emergency Stop Button Pressed"});
        }

        const bool systemPowerOffTriggered = HAL::MS::readBool(m_repo, POWER_OFF_BUTTON);
        // Dispatch power off event when: poweroff press is detected AND not already in poweroff state
        if (systemPowerOffTriggered &&
            !std::holds_alternative<StatePowerOff>(m_state) &&
            !std::holds_alternative<StatePreparePowerOff>(m_state))
        {
            dispatch(EvPowerOff{});
        }
    }

    void MasterFSM::stopAllServices()
    {
        // Immediate, non-blocking hardware halt across all Services.
        m_homingService->stop();
        m_drawerService->stop();
        m_stowageService->stop();
        m_alignmentService->stop();
        m_visionService->stop();
        m_contactService->stop();
        m_exposureService->stop();
    }

    void MasterFSM::updateAndBroadcastPosture(const ExpectedSystemPosture &expected, SystemPosture &current)
    {
        // Merge the optional expectations into the concrete current posture
        if (expected.hasValue())
        {
            current.merge(expected);
            emit s_postureChanged(current); // Fire signal to update subscribers on posture change
        }
    }

} // namespace Kub3::MFSM
