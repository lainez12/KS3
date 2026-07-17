#include <QThread>

#include <Controllers/ProcedureTestController.h>
#include <Services/Alignment/AlignmentService.h>
#include <Services/Contact/ContactService.h>
#include <Services/Exposure/ExposureService.h>
#include <Services/Homing/HomingService.h>
#include <Services/Stowage/StowageService.h>
#include <Services/Vision/VisionService.h>

#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
#include <Services/Drawers/SingleConveyorDrawerService.h>
#elif defined(KUB_MODEL_8)
#include <Services/Drawers/DualConveyorDrawerService.h>
#endif

constexpr uint64_t pack(uint32_t c, uint32_t s)
{
    return (static_cast<uint64_t>(c) << 32) | static_cast<uint64_t>(s);
}

namespace Kub3::Tools::Tester
{
#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
    using ConveyorDrawerService = Kub3::Services::SingleConveyorDrawerService;
#elif defined(KUB_MODEL_8)
    using ConveyorDrawerService = Kub3::Services::DualConveyorDrawerService;
#endif

    static Services::AlignmentDirection convert(AlignmentStageDirection d)
    {
        switch (d)
        {
        case AlignmentStageDirection::X_LEFT:
            return Services::AlignmentDirection::LEFT;
        case AlignmentStageDirection::X_RIGHT:
            return Services::AlignmentDirection::RIGHT;
        case AlignmentStageDirection::Y_BACK:
            return Services::AlignmentDirection::BACK;
        case AlignmentStageDirection::Y_FRONT:
            return Services::AlignmentDirection::FRONT;
        case AlignmentStageDirection::THETA_CW:
            return Services::AlignmentDirection::CLOCKWISE;
        case AlignmentStageDirection::THETA_CCW:
            return Services::AlignmentDirection::COUNTER_CLOCKWISE;
        }
        Q_UNREACHABLE();
    }

    ProcedureTestController::ProcedureTestController(Shared<HAL::Act::ActuatorRegistry> registry,
                                                     Shared<HAL::MS::IMachineStatusRepo> repo,
                                                     Config::process_config_t processConfig,
                                                     Config::hardware_config_t hwConfig,
                                                     QObject *parent) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo)),
        m_processConfig(processConfig),
        m_hwConfig(hwConfig),
        m_tickTimer(this),
        QObject(parent)
    {
        m_tickTimer.setInterval(20); // 50Hz bypass engine
        m_tickTimer.setTimerType(Qt::PreciseTimer);

        connect(&m_tickTimer, &QTimer::timeout, this, &ProcedureTestController::onTick);
    }

    ProcedureTestController::~ProcedureTestController()
    {
        stop();
    }

    void ProcedureTestController::start()
    {
        // Create services so they are in the same thread as the controller
        m_homingService    = std::make_shared<Services::HomingService>(m_registry, m_repo, m_processConfig, m_hwConfig);
        m_drawerService    = std::make_shared<ConveyorDrawerService>(m_registry, m_repo, m_processConfig);
        m_stowageService   = std::make_shared<Services::StowageService>(m_registry, m_repo, m_processConfig);
        m_alignmentService = std::make_shared<Services::AlignmentService>(m_registry, m_repo, m_processConfig);
        m_visionService    = std::make_shared<Services::VisionService>(m_registry, m_repo, m_processConfig);
        m_contactService   = std::make_shared<Services::ContactService>(m_registry, m_repo, m_processConfig, m_hwConfig);
        m_exposureService  = std::make_shared<Services::ExposureService>(m_registry);

        m_allServices = {
            m_homingService.get(),
            m_drawerService.get(),
            m_stowageService.get(),
            m_alignmentService.get(),
            m_visionService.get(),
            m_contactService.get(),
            m_exposureService.get()};
    }

    void ProcedureTestController::stop()
    {
        ps_emergencyStop();
    }

    // ==========================================
    // TICK ENGINE
    // ==========================================

    void ProcedureTestController::onTick()
    {
        if (!m_activeService)
        {
            m_tickTimer.stop();
            return;
        }

        if (m_activeService == m_visionService.get())
        {
            for (const auto &move : m_activeCameraMoves)
                m_visionService->moveManual(move.motor, move.dir); // Feed the watchdog for active movements
        }
        else if (m_activeService == m_alignmentService.get())
        {
            for (const auto &move : m_activeAlignmentStageMoves)
                m_alignmentService->moveStage(move.motor, move.dir); // Feed the watchdog for active movements
        }
        m_activeService->tick();

        const auto status = m_activeService->getStatus();

        if (status == Services::ServiceStatus::Success)
        {
            m_tickTimer.stop();
            m_activeService = nullptr;
            emit s_procedureCompleted(m_currentProcedureName);
        }
        else if (status == Services::ServiceStatus::Error)
        {
            m_tickTimer.stop();
            QString reason  = QString::fromStdString(m_activeService->getErrorReason());
            m_activeService = nullptr;
            emit s_procedureFailed(m_currentProcedureName, reason);
        }
    }

    void ProcedureTestController::startServiceRoutine(Services::IService *service, const QString &procedureName, bool noEmit)
    {
        if (m_activeService)
        {
            emit s_procedureFailed(procedureName, "Engine busy. Stop required first.");
            return;
        }

        m_currentProcedureName = procedureName;
        m_activeService        = service;

        if (noEmit == false)
            emit s_procedureStarted(m_currentProcedureName);
        m_tickTimer.start();
    }

    // ==========================================
    // INCOMING COMMANDS
    // ==========================================

    void ProcedureTestController::ps_emergencyStop()
    {
        qCritical() << "ProcedureTester: EMERGENCY STOP INITIATED.";

        m_activeCameraMoves.clear(); // Wipe active camera movements tracking
        for (Services::IService *service : m_allServices)
        {
            if (service)
                service->stop();
        }

        if (m_activeService)
        {
            QString abortedProc = m_currentProcedureName;

            m_activeService = nullptr;
            m_tickTimer.stop();
            emit s_procedureFailed(abortedProc, "ABORTED BY USER (Emergency Stop)");
        }
    }

    void ProcedureTestController::ps_runInitStages(void)
    {
        m_homingService->runGranularAction(Services::HomingTarget::ALIGNMENT_STAGES);
        startServiceRoutine(m_homingService.get(), "Stages Initialization Sequence");
    }

    void ProcedureTestController::ps_runCenterStages(void)
    {
        m_homingService->runGranularAction(Services::HomingTarget::ALIGNMENT_STAGES, false);
        startServiceRoutine(m_homingService.get(), "Stages Homing Sequence");
    }

    void ProcedureTestController::ps_runInitCameras(void)
    {
        m_homingService->runGranularAction(Services::HomingTarget::CAMERAS);
        startServiceRoutine(m_homingService.get(), "Cameras Initialization Sequence");
    }

    void ProcedureTestController::ps_runInitDeck(void)
    {
        m_homingService->runGranularAction(Services::HomingTarget::DECK);
        startServiceRoutine(m_homingService.get(), "Deck Initialization Sequence");
    }

    void ProcedureTestController::ps_runInitVision(void)
    {
        m_homingService->runGranularAction(
            static_cast<Services::HomingTarget::Type>(Services::HomingTarget::CAMERAS | Services::HomingTarget::DECK));
        startServiceRoutine(m_homingService.get(), "Vision Initialization Sequence");
    }

    void ProcedureTestController::ps_runHoming(int targetBits)
    {
        auto target = static_cast<Services::HomingTarget::Type>(targetBits);
        m_homingService->home(target);
        startServiceRoutine(m_homingService.get(), "Homing Sequence");
    }

    void ProcedureTestController::ps_runDrawerOperation(DrawerTarget target, bool eject)
    {
        if (eject)
            m_drawerService->eject(target);
        else
            m_drawerService->insert(target);

        const QString procName = QString((target == DrawerTarget::Mask) ? "Drawer Mask " : "Drawer Wafer ").append(eject ? "Ejection" : "Insertion");
        startServiceRoutine(m_drawerService.get(), procName);
    }

    void ProcedureTestController::ps_runStowage(int targetInt)
    {
        auto target = static_cast<Services::StowageTarget>(targetInt);
        m_stowageService->startStowage(target);
        startServiceRoutine(m_stowageService.get(), "Stowage Sequence");
    }

    void ProcedureTestController::ps_runUnstowage(int targetInt)
    {
        // Unstowage is routed via homing mechanically
        auto target = static_cast<Services::HomingTarget::Type>(targetInt);
        m_homingService->home(target);
        startServiceRoutine(m_homingService.get(), "Unstowage Sequence");
    }

    void ProcedureTestController::ps_runInitZAxes(void)
    {
        m_homingService->runGranularAction(Services::HomingTarget::Z_MOTORS);
        startServiceRoutine(m_homingService.get(), "Z Axes Initialization Sequence");
    }

    void ProcedureTestController::ps_runHomeZAxes(void)
    {
        m_homingService->runGranularAction(Services::HomingTarget::Z_MOTORS, false);
        startServiceRoutine(m_homingService.get(), "Z Axes Homing Sequence");
    }

    void ProcedureTestController::ps_runAutolevel(void)
    {
        m_contactService->startContactRoutine(Services::AutolevelingPayload{});
        startServiceRoutine(m_contactService.get(), "Autoleveling");
    }

    void ProcedureTestController::ps_toggleForceSensors(bool en)
    {
        m_contactService->toggleForceSensors(en);
    }

    void ProcedureTestController::ps_runCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir)
    {
        // Translates UI enums to Logic enums
        auto resolveHardware = [](CameraId c, CameraDirection d) -> std::pair<Services::VisionMotor, Services::VisionDirection> {
            if (c == CameraId::LEFT)
            {
                switch (d)
                {
                case CameraDirection::UP:
                    return {Services::VisionMotor::UpperLeftCameraY, Services::VisionDirection::UpperLeftCamYBack};
                case CameraDirection::DOWN:
                    return {Services::VisionMotor::UpperLeftCameraY, Services::VisionDirection::UpperLeftCamYFront};
                case CameraDirection::LEFT:
                    return {Services::VisionMotor::UpperLeftCameraX, Services::VisionDirection::UpperLeftCamXLeft};
                case CameraDirection::RIGHT:
                    return {Services::VisionMotor::UpperLeftCameraX, Services::VisionDirection::UpperLeftCamXRight};
                }
            }
            else if (c == CameraId::RIGHT)
            {
                switch (d)
                {
                case CameraDirection::UP:
                    return {Services::VisionMotor::UpperRightCameraY, Services::VisionDirection::UpperRightCamYBack};
                case CameraDirection::DOWN:
                    return {Services::VisionMotor::UpperRightCameraY, Services::VisionDirection::UpperRightCamYFront};
                case CameraDirection::LEFT:
                    return {Services::VisionMotor::UpperRightCameraX, Services::VisionDirection::UpperRightCamXLeft};
                case CameraDirection::RIGHT:
                    return {Services::VisionMotor::UpperRightCameraX, Services::VisionDirection::UpperRightCamXRight};
                }
            }

            Q_UNREACHABLE();
        };

        const auto [motor, direction] = resolveHardware(camId, dir);

        if (kind == MovementKind::STOP)
        {
            // Erase the movement from the tracking list
            m_activeCameraMoves.erase(
                std::remove_if(
                    m_activeCameraMoves.begin(),
                    m_activeCameraMoves.end(),
                    [motor](const CameraMovement &m) { return m.motor == motor; }),
                m_activeCameraMoves.end());
            m_visionService->stopManual(motor);

            // Detach VisionService from the Tick engine if it has nothing left to do
            if (m_activeCameraMoves.empty() &&
                m_activeService == m_visionService.get() &&
                m_visionService->getStatus() == Services::ServiceStatus::Idle)
            {
                m_activeService = nullptr;
                m_tickTimer.stop();
            }
        }
        else
        {
            // Reject manual pad movement if a real automated procedure is running
            if (m_activeService && m_activeService != m_visionService.get())
            {
                qWarning() << "Cannot move camera manually while an automated procedure is running.";
                return;
            }

            const bool isGranular = (kind == MovementKind::GRANULAR);

            if (!isGranular)
            {
                auto it = std::find_if(
                    m_activeCameraMoves.begin(),
                    m_activeCameraMoves.end(),
                    [motor](const CameraMovement &m) { return m.motor == motor; });

                // Insert the movement into our tracking list
                if (it != m_activeCameraMoves.end())
                    it->dir = direction;
                else
                    m_activeCameraMoves.push_back({motor, direction});

                // Attach VisionService to the Tick engine if it's currently completely idle
                startServiceRoutine(m_visionService.get(), "CameraPADMovement", true);
            }
            m_visionService->moveManual(motor, direction, isGranular); // Initiate movement
        }
    }

    void ProcedureTestController::ps_runAlignmentStageMovement(
        AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir)
    {
        const auto direction = convert(dir);

        if (kind == MovementKind::STOP)
        {
            // Erase the movement from the tracking list
            m_activeAlignmentStageMoves.erase(
                std::remove_if(
                    m_activeAlignmentStageMoves.begin(),
                    m_activeAlignmentStageMoves.end(),
                    [stageId](const AlignmentStageMovement &m) { return m.motor == stageId; }),
                m_activeAlignmentStageMoves.end());
            m_alignmentService->stopStage(stageId);

            // Detach AlignmentService from the Tick engine if it has nothing left to do
            if (m_activeAlignmentStageMoves.empty() &&
                m_activeService == m_alignmentService.get() &&
                m_alignmentService->getStatus() == Services::ServiceStatus::Idle)
            {
                m_activeService = nullptr;
                m_tickTimer.stop();
            }
        }
        else
        {
            // Reject manual pad movement if a real automated procedure is running
            if (m_activeService && m_activeService != m_alignmentService.get())
            {
                qWarning() << "Cannot move alignment stages manually while an automated procedure is running.";
                return;
            }

            const bool isGranular = (kind == MovementKind::GRANULAR);

            if (!isGranular)
            {
                auto it = std::find_if(
                    m_activeAlignmentStageMoves.begin(),
                    m_activeAlignmentStageMoves.end(),
                    [stageId](const AlignmentStageMovement &m) { return m.motor == stageId; });

                // Insert the movement into our tracking list
                if (it != m_activeAlignmentStageMoves.end())
                    it->dir = direction;
                else
                    m_activeAlignmentStageMoves.push_back({stageId, direction});

                // Attach Alignment to the Tick engine if it's currently completely idle
                startServiceRoutine(m_alignmentService.get(), "AlignmentStagePADMovement", true);
            }
            m_alignmentService->moveStage(stageId, direction, isGranular); // Initiate movement
        }
    }

} // namespace Kub3::Tools::Tester
