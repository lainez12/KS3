#include <QDebug>

#include <ApplicationBuilder.h>

// Services
#include <Services/Alignment/AlignmentService.h>
#include <Services/Contact/ContactService.h>
#include <Services/Homing/HomingService.h>
#include <Services/Vision/VisionService.h>
#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
#include <Services/Drawers/SingleConveyorDrawerService.h>
#elif defined(KUB_MODEL_8)
#include <Services/Drawers/DualConveyorDrawerService.h>
#endif

namespace Kub3
{

#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
    using ConveyorDrawerService = Services::SingleConveyorDrawerService;
#elif defined(KUB_MODEL_8)
    using ConveyorDrawerService = Services::DualConveyorDrawerService;
#endif

    ApplicationBuilder &ApplicationBuilder::loadConfigurations(const std::string &hwPath, const std::string &processPath)
    {
        qInfo() << "Loading Configurations...";
        m_hwConfig      = Config::ConfigLoader::loadHardwareConfig(hwPath);
        m_processConfig = Config::ConfigLoader::loadProcessConfig(processPath);

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::buildHardwareTier(void)
    {
        qInfo() << "Building Tier 3 (Hardware)...";
        m_repo = std::make_shared<HAL::MS::MachineStatusRepo>();

        // Strict Dependency Injection
        m_hwManager = std::make_unique<HAL::HardwareManager>(m_repo, m_hwConfig);

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::buildLogicTier(void)
    {
        qInfo() << "Building Tier 2 (Logic)...";

        auto registry = m_hwManager->getActuatorRegistry();

        m_drawerService    = std::make_shared<ConveyorDrawerService>(registry, m_repo, m_processConfig);
        m_homingService    = std::make_shared<Services::HomingService>(registry, m_repo, m_processConfig);
        m_visionService    = std::make_shared<Services::VisionService>(registry, m_repo, m_processConfig);
        m_contactService   = std::make_shared<Services::ContactService>(registry, m_repo, m_processConfig, m_hwConfig);
        m_alignmentService = std::make_shared<Services::AlignmentService>(registry, m_repo, m_processConfig);

        // Standard Qt Worker Object instantiation
        // Parented to qApp to ensure no memory leaks if run() is bypassed
        m_logicThread = new QThread(qApp);
        m_masterFSM   = new MFSM::MasterFSM(
            m_repo,
            m_homingService,
            m_drawerService,
            m_alignmentService,
            m_visionService,
            m_contactService);

        // Move the FSM to the logic thread
        m_masterFSM->moveToThread(m_logicThread);

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::buildUserInterfaceTier(void)
    {
        qInfo() << "Building Tier 1 (UI)...";

        m_mainWindow = std::make_unique<MainWindow>();

        // Building MachineStatusView
        {
            m_machineStatusVM       = std::make_shared<UI::ViewModels::MachineStatusViewModel>(m_repo);
            auto *machineStatusView = new MachineStatusView(m_machineStatusVM, m_mainWindow.get());

            m_mainWindow->addView(Kub3::UI::ViewId::MACHINE_STATUS_VIEW, machineStatusView);
        }

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::wireArchitecture(void)
    {
        qInfo() << "Wiring Inter-Tier Connections...";

        namespace VM = UI::ViewModels;

        // Thread Lifecycle Wiring
        QObject::connect(m_logicThread, &QThread::started, m_masterFSM, &MFSM::MasterFSM::start);

        // Logic -> HAL Wiring
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_requestHardwareRetry, m_hwManager.get(), &HAL::HardwareManager::ps_reconnectMCUSubsystem);
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_requestPowerOff, m_hwManager.get(), &HAL::HardwareManager::ps_powerOff);
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_requestCameraParamUpdate, m_hwManager.get(), &HAL::HardwareManager::ps_updateCameraParameter);

        // System power-off
        QObject::connect(m_hwManager.get(), &HAL::HardwareManager::s_hardwarePowerOffSent, [this]() { this->powerOff(); });
        // Graceful Shutdown
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_logicThread, &QThread::quit);
        QObject::connect(m_logicThread, &QThread::finished, m_masterFSM, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_logicThread, &QObject::deleteLater);

        // 2. UI -> Logic Wiring (Queued Connections implicitly used across threads)
        auto *msvm = m_machineStatusVM.get();

        QObject::connect(m_mainWindow.get(), &MainWindow::s_initializationRequest, m_masterFSM, &MFSM::MasterFSM::ps_requestInitialization);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_exposureSliderValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestExposureUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_gainSliderValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestGainUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_framerateValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestFrameRateUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_centeredZoomValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestCenteredZoomUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_roiChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestROIUpdate);

        // 3. Logic -> UI Wiring
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_stateChanged, m_mainWindow.get(), &MainWindow::ps_stateChanged);
        m_machineStatusVM->bindConnection(m_hwManager.get(), &HAL::HardwareManager::s_cameraFrameReady,
                                          msvm, &VM::BaseVisionViewModel::ps_onCameraFrameReceived);
        m_machineStatusVM->bindConnection(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_sensorValueChanged,
                                          msvm, &VM::MachineStatusViewModel::ps_handleSensorValueChanged);

        return *this;
    }

    int ApplicationBuilder::run(QApplication &app)
    {
        qInfo() << "[ApplicationBuilder::run]: Starting Hardware Manager.";
        m_hwManager->startAll();

        qInfo() << "[ApplicationBuilder::run]: Starting Logic Thread.";
        m_logicThread->start();

        qInfo() << "[ApplicationBuilder::run]: Showing UI.";
        m_mainWindow->ps_openView(Kub3::UI::ViewId::MACHINE_STATUS_VIEW); // Open initial view
#if defined(BUILD_DEBUG)
        m_mainWindow->show();
#else
        m_mainWindow->showFullScreen();
#endif

        // Blocking call to main Qt Event Loop
        const int ret = app.exec();

        qInfo() << "Shutting down Hardware Manager.";
        m_hwManager->stopAll();

        return ret;
    }

    void ApplicationBuilder::powerOff(void)
    {
#ifdef BUILD_DEBUG
        qDebug() << "[ApplicationBuilder::powerOff] triggered (debug mode: closing app).";
        qApp->quit();
#else
        if (std::system("sudo poweroff") != 0)
            qCritical() << "Failed to run power off command.";
#endif
    }

} // namespace Kub3
