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
            auto machineStatusViewModel            = std::make_unique<UI::ViewModels::MachineStatusViewModel>(m_repo);
            auto homeViewModel                     = std::make_unique<UI::ViewModels::HomeViewModel>(m_repo);
            auto settingsViewModel                 = std::make_unique<UI::ViewModels::SettingsViewModel>(m_repo);
            auto saveExposureSettingsViewModel     = std::make_unique<UI::ViewModels::ViewModelsExposure::SaveExposureSettingsViewModel>(m_repo);
            auto exposureSettingsViewModel         = std::make_unique<UI::ViewModels::ViewModelsExposure::ExposureSettingsViewModel>(m_repo);
            auto favoriteExposureSettingsViewModel = std::make_unique<UI::ViewModels::ViewModelsExposure::FavoriteExposureSettingsViewModel>(m_repo);
            auto recapExposureSettingsViewModel    = std::make_unique<UI::ViewModels::ViewModelsExposure::RecapExposureSettingsViewModel>(m_repo);
            auto progressExposureViewModel         = std::make_unique<UI::ViewModels::ViewModelsExposure::ProgressExposureViewModel>(m_repo);
            auto completeExposureViewModel         = std::make_unique<UI::ViewModels::ViewModelsExposure::CompleteExposureViewModel>(m_repo);
            auto versionViewModel                  = std::make_unique<UI::ViewModels::ViewModelsSettings::VersionViewModel>(m_repo);
            auto temperatureViewModel              = std::make_unique<UI::ViewModels::ViewModelsSettings::TemperatureViewModel>(m_repo);
            auto operatingTimeViewModel            = std::make_unique<UI::ViewModels::ViewModelsSettings::OperatingTimesViewModel>(m_repo);
            auto screenshotViewModel               = std::make_unique<UI::ViewModels::ViewModelsSettings::ScreenshotExportViewModel>(m_repo);
            auto ledTestViewModel                  = std::make_unique<UI::ViewModels::ViewModelsSettings::LedTestViewModel>(m_repo);
            auto updateSoftwareViewModel           = std::make_unique<UI::ViewModels::ViewModelsSettings::UpdateSoftwareViewModel>(m_repo);
            auto distanceViewModel                 = std::make_unique<UI::ViewModels::ViewModelsAlignment::DistanceViewModel>(m_repo);
            auto visualisationViewModel            = std::make_unique<UI::ViewModels::ViewModelsAlignment::VisualisationViewModel>(m_repo);
            auto loadParametersViewModel           = std::make_unique<UI::ViewModels::ViewModelsAlignment::LoadParametersViewModel>(m_repo);
            auto saveParametersViewModel           = std::make_unique<UI::ViewModels::ViewModelsAlignment::SaveParametersViewModel>(m_repo);
            auto *settingsView                     = new SettingsView(std::move(settingsViewModel), m_mainWindow.get());
            auto *exposureSettingsView             = new ExposureSettingsView(std::move(exposureSettingsViewModel), m_mainWindow.get());
            auto *favoriteExposureSettingsView     = new FavoriteExposureSettingsView(std::move(favoriteExposureSettingsViewModel), m_mainWindow.get());
            auto *recapExposureSettingsView        = new RecapExposureSettingsView(std::move(recapExposureSettingsViewModel), m_mainWindow.get());
            auto *progressExposureView             = new ProgressExposureView(std::move(progressExposureViewModel), m_mainWindow.get());
            auto *completeExposureView             = new CompleteExposureView(std::move(completeExposureViewModel), m_mainWindow.get());
            auto *versionView                      = new VersionView(std::move(versionViewModel), m_mainWindow.get());
            auto *temperatureView                  = new TemperatureView(std::move(temperatureViewModel), m_mainWindow.get());
            auto *operatingTimeView                = new OperatingTimesView(std::move(operatingTimeViewModel), m_mainWindow.get());
            auto *screenshotView                   = new ScreenshotExportView(std::move(screenshotViewModel), m_mainWindow.get());
            auto *ledTestView                      = new LedTestView(std::move(ledTestViewModel), m_mainWindow.get());
            auto *machineStatusView                = new MachineStatusView(std::move(machineStatusViewModel), m_mainWindow.get());
            auto *saveExposureSettingsView         = new SaveExposureSettingsView(std::move(saveExposureSettingsViewModel), m_mainWindow.get());
            auto *updateSoftwareView               = new UpdateSoftwareView(std::move(updateSoftwareViewModel), m_mainWindow.get());
            auto *distanceView                     = new DistanceView(std::move(distanceViewModel), m_mainWindow.get());
            auto *visualisationView                = new VisualisationView(std::move(visualisationViewModel), m_mainWindow.get());
            auto *loadParametersView               = new LoadParametersView(std::move(loadParametersViewModel), m_mainWindow.get());
            auto *saveParametersView               = new SaveParametersView(std::move(saveParametersViewModel), m_mainWindow.get());
            auto *homeView                         = new HomeView(std::move(homeViewModel), m_mainWindow.get());
            auto *homeEightView                    = new HomeEightView(std::make_unique<UI::ViewModels::HomeViewModel>(m_repo), m_mainWindow.get());
            m_mainWindow->addView(Kub3::UI::ViewId::HOME_VIEW, homeView);
            m_mainWindow->addView(Kub3::UI::ViewId::HOME_EIGHT_VIEW, homeEightView);
            m_mainWindow->addView(Kub3::UI::ViewId::MACHINE_STATUS_VIEW, machineStatusView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_VIEW, settingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW, exposureSettingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::FAVORITE_EXPOSURE_SETTINGS_VIEW, favoriteExposureSettingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::RECAP_EXPOSURE_SETTINGS_VIEW, recapExposureSettingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::PROGRESS_EXPOSURE_VIEW, progressExposureView);
            m_mainWindow->addView(Kub3::UI::ViewId::COMPLETE_EXPOSURE_VIEW, completeExposureView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_ABOUT_VIEW, versionView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_TEMPERATURE_VIEW, temperatureView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_OPERATING_TIMES_VIEW, operatingTimeView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_SCREENSHOT_EXPORT_VIEW, screenshotView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_LED_TEST_VIEW, ledTestView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_UPDATE_SOFTWARE_VIEW, updateSoftwareView);
            m_mainWindow->addView(Kub3::UI::ViewId::SAVE_EXPOSURE_SETTINGS_VIEW, saveExposureSettingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_DISTANCE_VIEW, distanceView);
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW, visualisationView);
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_LOAD_PARAMETERS_VIEW, loadParametersView);
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_SAVE_PARAMETERS_VIEW, saveParametersView);
        }

        if (m_masterFSM) {
            QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_stateChanged, m_mainWindow.get(), &MainWindow::ps_stateChanged);
        }

        m_mainWindow->ps_openView(Kub3::UI::ViewId::HOME_EIGHT_VIEW);

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
