#include <QDebug>

#include <Common/Enums.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <MFSM/MasterFSM.h>
#include <MFSM/events.h>
#include <Services/Contact/IContactService.h>
#include <Services/Vision/VisionService.h>

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
        // Telemetry router
        auto logRouter = [this](LogLevel level, const std::string &msg) {
            Common::ProcessMessageLevel mappedLevel;
            switch (level)
            {
            case LogLevel::Info:
                mappedLevel = Common::ProcessMessageLevel::Info;
                break;
            case LogLevel::Success:
                mappedLevel = Common::ProcessMessageLevel::Success;
                break;
            case LogLevel::Warning:
                mappedLevel = Common::ProcessMessageLevel::Warning;
                break;
            case LogLevel::Error:
                mappedLevel = Common::ProcessMessageLevel::Error;
                break;
            }

            emit s_processMessageBroadcast(Common::ProcessMessage{
                .text  = QString::fromStdString(msg),
                .level = mappedLevel,
            });
        };

        m_homingService->setLogCallback(logRouter);
        m_drawerService->setLogCallback(logRouter);
        m_stowageService->setLogCallback(logRouter);
        m_alignmentService->setLogCallback(logRouter);
        m_visionService->setLogCallback(logRouter);
        m_contactService->setLogCallback(logRouter);
        m_exposureService->setLogCallback(logRouter);

        // Hard-wire the FSM Logic Loop every LOGIC_TIMER_PERIOD_MS milliseconds
        // Zero Thread Blocking: The logic loop must never sleep/wait.
        m_logicTimer.setInterval(LOGIC_TIMER_PERIOD_MS);
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
#if defined(__GNUC__) && __GNUC__ >= 12 && !defined(__clang__)
/**
 * @brief GCC 12+ False Positive Suppression
 *
 * GCC's optimizer incorrectly tracks active variant union members during operator=,
 * erroneously reporting an invalid 'free-nonheap-object' when destroying std::string members.
 * We temporarily disable the warning locally to allow clean variant assignment.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif
        m_state = nextState;
#if defined(__GNUC__) && __GNUC__ >= 12 && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
        if (macroChanged || microChanged)
        {
            if (macroChanged)
            {
                // The system state changed (e.g., Booting -> Init, or Operational -> Fault)
                onStateEntered(m_state);
            }
            else if (microChanged)
            {
                // A focused operation change (e.g., Idle -> DrawerOp)
                const auto &opState = std::get<StateOperational>(m_state);
                onOperationalSubstateEntered(opState, opState.subState);
            }
        }
    }

    // ==========================================================================
    // TIER 1 (UI) -> TIER 2 (LOGIC) PUBLIC SLOTS
    // ==========================================================================
    // These slots securely package primitive UI commands into type-safe SystemEvents.

    void MasterFSM::ps_requestRetryBoot(void)
    {
        dispatch(CmdRetryBoot{});
    }

    void MasterFSM::ps_requestAbortOperation(void)
    {
        dispatch(CmdAbortOperation{});
    }

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

    void MasterFSM::ps_requestOperateDrawer(DrawerTarget tgt, bool eject)
    {
        dispatch(CmdOperateDrawer{
            .target    = tgt,
            .operation = eject ? DrawerOperation::EJECT : DrawerOperation::INSERT,
        });
    }

    void MasterFSM::ps_requestStowage(StowageTarget tgt)
    {
        dispatch(CmdOperateStowage{
            .target = tgt,
        });
    }

    void MasterFSM::ps_requestAutolevel()
    {
        dispatch(CmdStartAutolevel{});
    }

    void MasterFSM::ps_requestEnterAlignment()
    {
        dispatch(CmdEnterAlignmentMode{.autoMode = false});
    }

    void MasterFSM::ps_requestApplyContact(double forceGF)
    {
        dispatch(CmdApplyContact{.forceGF = forceGF});
    }

    void MasterFSM::ps_requestSubstrateCompressedAir(bool enable)
    {
        dispatch(CmdSetSubstrateCompressedAir{.enableCompressedAir = enable});
    }

    void MasterFSM::ps_requestEnterExposureMode()
    {
        dispatch(CmdEnterExposureMode{});
    }

    void MasterFSM::ps_requestUnstowage(StowageTarget tgt)
    {
        dispatch(CmdOperateUnstowage{
            .target = tgt,
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

    void MasterFSM::ps_requestPADCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir)
    {
        using namespace Services;

        struct Mapping {
            VisionMotor motor;
            VisionDirection dir;
        };

        const auto getMapping = [](CameraId cam, CameraDirection d) -> Mapping {
            if (cam == CameraId::LEFT)
            {
                switch (d)
                {
                case CameraDirection::UP:
                    return {VisionMotor::UpperLeftCameraY, VisionDirection::UpperLeftCamYBack};
                case CameraDirection::DOWN:
                    return {VisionMotor::UpperLeftCameraY, VisionDirection::UpperLeftCamYFront};
                case CameraDirection::LEFT:
                    return {VisionMotor::UpperLeftCameraX, VisionDirection::UpperLeftCamXLeft};
                case CameraDirection::RIGHT:
                    return {VisionMotor::UpperLeftCameraX, VisionDirection::UpperLeftCamXRight};
                }
            }
            else if (cam == CameraId::RIGHT)
            {
                switch (d)
                {
                case CameraDirection::UP:
                    return {VisionMotor::UpperRightCameraY, VisionDirection::UpperRightCamYBack};
                case CameraDirection::DOWN:
                    return {VisionMotor::UpperRightCameraY, VisionDirection::UpperRightCamYFront};
                case CameraDirection::LEFT:
                    return {VisionMotor::UpperRightCameraX, VisionDirection::UpperRightCamXLeft};
                case CameraDirection::RIGHT:
                    return {VisionMotor::UpperRightCameraX, VisionDirection::UpperRightCamXRight};
                }
            }
            Q_UNREACHABLE();
        };

        const Mapping target = getMapping(camId, dir);

        if (kind == MovementKind::STOP)
        {
            dispatch(CmdVisualisation{
                .targetMotor = target.motor,
                .operation   = VisionStopPayload{},
            });
        }
        else
        {
            dispatch(CmdVisualisation{
                .targetMotor = target.motor,
                .operation   = VisionMovePayload{
                      .dir      = target.dir,
                      .granular = (kind == MovementKind::GRANULAR),
                },
            });
        }
    }

    void MasterFSM::ps_requestPADAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir)
    {
        using namespace Services;

        const auto convert = [](AlignmentStageDirection d) -> AlignmentDirection {
            switch (d)
            {
            case AlignmentStageDirection::X_LEFT:
                return AlignmentDirection::LEFT;
            case AlignmentStageDirection::X_RIGHT:
                return AlignmentDirection::RIGHT;
            case AlignmentStageDirection::Y_BACK:
                return AlignmentDirection::BACK;
            case AlignmentStageDirection::Y_FRONT:
                return AlignmentDirection::FRONT;
            case AlignmentStageDirection::THETA_CW:
                return AlignmentDirection::CLOCKWISE;
            case AlignmentStageDirection::THETA_CCW:
                return AlignmentDirection::COUNTER_CLOCKWISE;
            }
            Q_UNREACHABLE();
        };

        if (kind == MovementKind::STOP)
        {
            dispatch(CmdAlignmentPad{
                .stageId   = stageId,
                .operation = AlignmentStopStagePayload{},
            });
        }
        else
        {
            dispatch(CmdAlignmentPad{
                .stageId   = stageId,
                .operation = AlignmentMoveStagePayload{
                    .dir      = convert(dir),
                    .granular = (kind == MovementKind::GRANULAR),
                },
            });
        }
    }

    void MasterFSM::ps_requestPADZMovement(MovementKind kind, ZDirection dir)
    {
        if (kind == MovementKind::GRANULAR || kind == MovementKind::CONTINUOUS)
        {
            dispatch(CmdZAxisPad{
                .operation = Services::ZMovePayload{
                    .direction = dir,
                    .granular  = kind == MovementKind::GRANULAR,
                },
            });
        }
        else
        {
            dispatch(CmdZAxisPad{.operation = Services::ZStopPayload{}});
        }
    }

    void MasterFSM::ps_requestAlignmentSubstrateFineMode(bool active)
    {
        using namespace Services;

        const auto dispatchForStage = [this, active](AlignmentStageId id) {
            dispatch(CmdAlignmentPad{
                .stageId   = id,
                .operation = AlignmentSetKinematicModePayload{.fineMode = active},
            });
        };

        dispatchForStage(AlignmentStageId::X);
        dispatchForStage(AlignmentStageId::Y);
        dispatchForStage(AlignmentStageId::THETA);
    }

    void MasterFSM::ps_requestAlignmentCameraFineMode(CameraId camId, bool active)
    {
        using namespace Services;

        const auto dispatchEventForCamera = [this, active](VisionMotor motor) {
            dispatch(CmdVisualisation{
                .targetMotor = motor,
                .operation   = Services::VisionSetKinematicModePayload{.fineMode = active},
            });
        };

        if (camId == CameraId::LEFT)
        {
            dispatchEventForCamera(VisionMotor::UpperLeftCameraX);
            dispatchEventForCamera(VisionMotor::UpperLeftCameraY);
        }
        else if (camId == CameraId::RIGHT)
        {
            dispatchEventForCamera(VisionMotor::UpperRightCameraX);
            dispatchEventForCamera(VisionMotor::UpperRightCameraY);
        }
    }

    void MasterFSM::ps_requestAlignmentCameraAbsoluteMovement(CameraId camId, double xPosMm, double yPosMm)
    {
        using namespace Services;

        VisionMotor xMotor;
        VisionMotor yMotor;

        if (camId == CameraId::LEFT)
        {
            xMotor = VisionMotor::UpperLeftCameraX;
            yMotor = VisionMotor::UpperLeftCameraY;
        }
        else if (camId == CameraId::RIGHT)
        {
            xMotor = VisionMotor::UpperRightCameraX;
            yMotor = VisionMotor::UpperRightCameraY;
        }
        else
        {
            return;
        }

        // Dispatch Absolute Movement for X-Axis
        dispatch(CmdVisualisation{
            .targetMotor = xMotor,
            .operation   = VisionMoveAbsolutePayload{.positionMm = xPosMm},
        });
        // Dispatch Absolute Movement for Y-Axis
        dispatch(CmdVisualisation{
            .targetMotor = yMotor,
            .operation   = VisionMoveAbsolutePayload{.positionMm = yPosMm},
        });
    }

    // ==========================================================================
    // PAD & HARDWARE SERVICE ROUTING
    // ==========================================================================

    void MasterFSM::processCmdAlignmentPad(const CmdAlignmentPad &cmd)
    {
        if (auto *op = std::get_if<Services::AlignmentMoveStagePayload>(&cmd.operation))
            m_alignmentService->moveStage(cmd.stageId, op->dir, op->granular);
        else if (auto *op = std::get_if<Services::AlignmentStopStagePayload>(&cmd.operation))
            m_alignmentService->stopStage(cmd.stageId);
        else if (auto *op = std::get_if<Services::AlignmentSetKinematicModePayload>(&cmd.operation))
            m_alignmentService->setKinematicProfile(cmd.stageId, op->fineMode);
    }

    void MasterFSM::processCmdZPad(const CmdZAxisPad &cmd)
    {
        if (auto *op = std::get_if<Services::ZMovePayload>(&cmd.operation))
            m_contactService->moveZManual(op->direction, op->granular);
        else if (auto *op = std::get_if<Services::ZStopPayload>(&cmd.operation))
            m_contactService->stopZManual();
    }

    void MasterFSM::processCmdVisualisation(const CmdVisualisation &cmd)
    {
        if (auto *op = std::get_if<Services::VisionMovePayload>(&cmd.operation))
            m_visionService->moveManual(cmd.targetMotor, op->dir, op->granular);
        else if (auto *op = std::get_if<Services::VisionMoveAbsolutePayload>(&cmd.operation))
            m_visionService->moveAbsolute(cmd.targetMotor, op->positionMm);
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

    void MasterFSM::setCompressedAirAuthorized(bool authorized)
    {
        if (m_compressedAirAuthorized != authorized)
        {
            m_compressedAirAuthorized = authorized;
            emit s_compressedAirAuthorizedChanged(m_compressedAirAuthorized);
        }
    }

} // namespace Kub3::MFSM
