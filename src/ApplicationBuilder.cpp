#include <QDebug>

#include <ApplicationBuilder.h>

// Services
#include <Services/Homing/HomingService.h>
#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
#include <Services/Drawers/SingleConveyorDrawerService.h>
#elif defined(KUB_MODEL_8)
#include <Services/Drawers/DualConveyorDrawerService.h>
#endif

namespace Kub3 {

#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
    using ConveyorDrawerService = Services::SingleConveyorDrawerService;
#elif defined(KUB_MODEL_8)
    using ConveyorDrawerService = Services::DualConveyorDrawerService;
#endif

    ApplicationBuilder &ApplicationBuilder::loadConfigurations(const std::string &hwPath, const std::string &processPath) {
        qInfo() << "Loading Configurations...";
        m_hwConfig      = Config::ConfigLoader::loadHardwareConfig(hwPath);
        m_processConfig = Config::ConfigLoader::loadProcessConfig(processPath);

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::buildHardwareTier(void) {
        qInfo() << "Building Tier 3 (Hardware)...";
        m_repo = std::make_shared<HAL::MS::MachineStatusRepo>();

        // Strict Dependency Injection
        m_hwManager = std::make_unique<HAL::HardwareManager>(m_repo, m_hwConfig);

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::buildLogicTier(void) {
        qInfo() << "Building Tier 2 (Logic)...";

        m_drawerService = std::make_shared<ConveyorDrawerService>(
            m_hwManager->getActuatorRegistry(),
            m_repo,
            m_processConfig);

        m_homingService = std::make_shared<Services::HomingService>(
            m_hwManager->getActuatorRegistry(),
            m_repo,
            m_processConfig);

        // Standard Qt Worker Object instantiation
        // Parented to qApp to ensure no memory leaks if run() is bypassed
        m_logicThread = new QThread(qApp);
        m_masterFSM   = new MFSM::MasterFSM(m_repo, m_homingService, m_drawerService);

        // Move the FSM to the logic thread
        m_masterFSM->moveToThread(m_logicThread);

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::buildUserInterfaceTier(void) {
        qInfo() << "Building Tier 1 (UI)...";

        m_mainWindow = std::make_unique<MainWindow>();

        {
            auto machineStatusViewModel = std::make_unique<UI::ViewModels::MachineStatusViewModel>(m_repo);
            auto homeViewModel          = std::make_unique<UI::ViewModels::HomeViewModel>(m_repo);
            auto settingsViewModel      = std::make_unique<UI::ViewModels::SettingsViewModel>(m_repo);
            auto *settingsView          = new SettingsView(std::move(settingsViewModel), m_mainWindow.get());
            auto *machineStatusView = new MachineStatusView(std::move(machineStatusViewModel), m_mainWindow.get());
            auto *homeView          = new HomeView(std::move(homeViewModel), m_mainWindow.get());
            m_mainWindow->addView(Kub3::UI::ViewId::HOME_VIEW, homeView);
            m_mainWindow->addView(Kub3::UI::ViewId::MACHINE_STATUS_VIEW, machineStatusView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_VIEW, settingsView);
        }

        if (m_masterFSM) {
            QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_stateChanged, m_mainWindow.get(), &MainWindow::ps_stateChanged);
        }

        m_mainWindow->ps_openView(Kub3::UI::ViewId::HOME_VIEW);

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::wireArchitecture(void) {
        qInfo() << "Wiring Inter-Tier Connections...";

        // Thread Lifecycle Wiring
        QObject::connect(m_logicThread, &QThread::started, m_masterFSM, &MFSM::MasterFSM::start);

        // Logic -> HAL Wiring
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_requestHardwareRetry, m_hwManager.get(), &HAL::HardwareManager::ps_reconnectSubsystem);
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_requestPowerOff, m_hwManager.get(), &HAL::HardwareManager::ps_powerOff);

        // System power-off
        QObject::connect(m_hwManager.get(), &HAL::HardwareManager::s_hardwarePowerOffSent, [this]() { this->powerOff(); });
        // Graceful Shutdown
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_logicThread, &QThread::quit);
        QObject::connect(m_logicThread, &QThread::finished, m_masterFSM, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_logicThread, &QObject::deleteLater);

        // 2. UI -> Logic Wiring (Queued Connections implicitly used across threads)
        QObject::connect(m_mainWindow.get(), &MainWindow::s_initializationRequest, m_masterFSM, &MFSM::MasterFSM::ps_requestInitialization);

        // 3. Logic -> UI Wiring
        // QObject::connect(m_masterFSM, &MSFM::MasterFSM::stateChanged, m_mainWindow.get(), &MainWindow::onMachineStateChanged);

        return *this;
    }

    int ApplicationBuilder::run(QApplication &app) {
        qInfo() << "[ApplicationBuilder::run]: Starting Hardware Manager.";
        m_hwManager->startAll();

        qInfo() << "[ApplicationBuilder::run]: Starting Logic Thread.";
        m_logicThread->start();

        qInfo() << "[ApplicationBuilder::run]: Showing UI.";
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

    void ApplicationBuilder::powerOff(void) {
#ifdef BUILD_DEBUG
        qDebug() << "[ApplicationBuilder::powerOff] triggered (debug mode: closing app).";
        qApp->quit();
#else
        std::system("sudo poweroff");
#endif
    }

} // namespace Kub3
