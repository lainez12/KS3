#include <QDebug>

#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <MFSM/MasterFSM.h>
#include <Services/Contact/ContactService.h>

namespace Kub3::MFSM
{

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
        m_state(StateBooting{}),
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
        connect(&m_logicTimer, &QTimer::timeout, this, &MasterFSM::onLogicTick);
    }

    void MasterFSM::start(void)
    {
        m_logicTimer.start(20);         // 50Hz
        emit s_stateChanged("BOOTING"); // Signal the initial state to the UI
    }

    // FSM TRANSITION DISPATCHER
    void MasterFSM::dispatch(const SystemEvent &event)
    {
        const bool eventProcessed = processStaticEvent(m_state, event);

        if (eventProcessed)
            return;

        // Calculate the next state based on current state + event
        SystemState nextState = MasterFSM::processTransition(m_state, event);

        // If the state changed, apply it and trigger entry actions
        if (nextState.index() != m_state.index())
        {
            m_state = nextState;
            onStateEntered(m_state);
        }
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

    void MasterFSM::ps_requestStowage(void)
    {
        dispatch(CmdOperateStowage{});
    }

    void MasterFSM::ps_requestExposure(const Services::ExposurePayload &payload)
    {
        dispatch(CmdStartExposure{.payload = payload});
    }

} // namespace Kub3::MFSM
