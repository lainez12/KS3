#include <QDebug>
#include <QThread>

#include <Services/Alignment/AlignmentService.h>
#include <Services/Contact/ContactService.h>
#include <Services/Exposure/ExposureService.h>
#include <Services/Homing/HomingService.h>
#include <Services/Stowage/StowageService.h>
#include <Services/Vision/VisionService.h>
#include <controllers/ProcedureTestController.h>

#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
#include <Services/Drawers/SingleConveyorDrawerService.h>
#elif defined(KUB_MODEL_8)
#include <Services/Drawers/DualConveyorDrawerService.h>
#endif

namespace Kub3::Tools::Tester
{
#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
    using ConveyorDrawerService = Kub3::Services::SingleConveyorDrawerService;
#elif defined(KUB_MODEL_8)
    using ConveyorDrawerService = Kub3::Services::DualConveyorDrawerService;
#endif

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
        m_homingService    = std::make_shared<Services::HomingService>(m_registry, m_repo, m_processConfig);
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

    void ProcedureTestController::startServiceRoutine(Services::IService *service, const QString &procedureName)
    {
        if (m_activeService)
        {
            emit s_procedureFailed(procedureName, "Engine busy. Stop required first.");
            return;
        }

        m_currentProcedureName = procedureName;
        m_activeService        = service;

        emit s_procedureStarted(m_currentProcedureName);
        m_tickTimer.start();
    }

    // ==========================================
    // INCOMING COMMANDS
    // ==========================================

    void ProcedureTestController::ps_emergencyStop()
    {
        qCritical() << "ProcedureTester: EMERGENCY STOP INITIATED.";

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

    void ProcedureTestController::ps_runInitCameras(void)
    {
        m_homingService->initializeGranular(Services::HomingTarget::CAMERAS);
        startServiceRoutine(m_homingService.get(), "Cameras Initialization Sequence");
    }

    void ProcedureTestController::ps_runInitDeck(void)
    {
        m_homingService->initializeGranular(Services::HomingTarget::DECK);
        startServiceRoutine(m_homingService.get(), "Deck Initialization Sequence");
    }

    void ProcedureTestController::ps_runInitVision(void)
    {
        m_homingService->initializeGranular(
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

    void ProcedureTestController::ps_runAutolevel(void)
    {
        m_contactService->startContactRoutine(Services::AutolevelingPayload{});
        startServiceRoutine(m_contactService.get(), "Autoleveling");
    }

} // namespace Kub3::Tools::Tester
