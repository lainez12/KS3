#include <QDebug>

#include <ApplicationBuilder.h>

#include <Config/helper.h>
// Services
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

#define ASSIGN_VIEW_MODEL(Type, local_var, member_var, ...) \
    auto local_var   = std::make_unique<Type>(__VA_ARGS__); \
    this->member_var = local_var.get()

namespace Kub3
{

#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
    using ConveyorDrawerService = Services::SingleConveyorDrawerService;
#elif defined(KUB_MODEL_8)
    using ConveyorDrawerService = Services::DualConveyorDrawerService;
#endif

    ApplicationBuilder &ApplicationBuilder::loadConfigurations(const std::string &hwPath,
                                                               const std::string &processPath,
                                                               const std::string &adminPath)
    {
        qInfo() << "Loading Configurations...";
        m_hwConfig      = Config::ConfigLoader::loadHardwareConfig(hwPath);
        m_processConfig = Config::ConfigLoader::loadProcessConfig(processPath);
        m_adminConfig   = Config::ConfigLoader::loadAdminConfig(adminPath);

#if defined(BUILD_RELEASE)
        if (!m_adminConfig.kloe_mode)
        {
            QApplication::setOverrideCursor(Qt::BlankCursor);
        }
#endif

        qInfo() << m_hwConfig;
        qInfo() << m_processConfig;

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

        auto actReg = m_hwManager->getActuatorRegistry();

        m_homingService    = std::make_shared<Services::HomingService>(actReg, m_repo, m_processConfig);
        m_drawerService    = std::make_shared<ConveyorDrawerService>(actReg, m_repo, m_processConfig);
        m_stowageService   = std::make_shared<Services::StowageService>(actReg, m_repo, m_processConfig);
        m_alignmentService = std::make_shared<Services::AlignmentService>(actReg, m_repo, m_processConfig);
        m_visionService    = std::make_shared<Services::VisionService>(actReg, m_repo, m_processConfig);
        m_contactService   = std::make_shared<Services::ContactService>(actReg, m_repo, m_processConfig, m_hwConfig);
        m_exposureService  = std::make_shared<Services::ExposureService>(actReg);

        // Standard Qt Worker Object instantiation
        // Parented to qApp to ensure no memory leaks if run() is bypassed
        m_logicThread = new QThread(qApp);
        m_masterFSM   = new MFSM::MasterFSM(
            m_repo,
            m_homingService,
            m_drawerService,
            m_stowageService,
            m_alignmentService,
            m_visionService,
            m_contactService,
            m_exposureService);

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
            // View models creation
            ASSIGN_VIEW_MODEL(UI::ViewModels::HomeViewModel, homeVM, m_homeVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::HomeViewModel, homeEightVM, m_homeEightVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::MachineStatusViewModel, machineStatusVM, m_machineStatusVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::SettingsViewModel, settingsVM, m_settingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::ExposureSettingsViewModel, exposureSettingsVM, m_exposureSettingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::FavoriteExposureSettingsViewModel, favoriteExposureSettingsVM, m_favoriteExposureSettingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::RecapExposureSettingsViewModel, recapExposureSettingsVM, m_recapExposureSettingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::ProgressExposureViewModel, progressExposureVM, m_progressExposureVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::CompleteExposureViewModel, completeExposureVM, m_completeExposureVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::SaveExposureSettingsViewModel, saveExposureSettingsVM, m_saveExposureSettingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::VersionViewModel, versionVM, m_versionVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::TemperatureViewModel, temperatureVM, m_temperatureVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::OperatingTimesViewModel, operatingTimeVM, m_operatingTimeVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::ScreenshotExportViewModel, screenshotVM, m_screenshotVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::LedTestViewModel, ledTestVM, m_ledTestVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::UpdateSoftwareViewModel, updateSoftwareVM, m_updateSoftwareVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::DistanceViewModel, distanceVM, m_distanceVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::VisualisationViewModel, visualisationVM, m_visualisationVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::LoadParametersViewModel, loadParametersVM, m_loadParametersVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::SaveParametersViewModel, saveParametersVM, m_saveParametersVM, m_repo);

            // Views creation
            auto *homeView                     = new HomeView(std::move(homeVM), m_mainWindow.get());
            auto *homeEightView                = new HomeEightView(std::move(homeEightVM), m_mainWindow.get());
            auto *machineStatusView            = new MachineStatusView(std::move(machineStatusVM), m_mainWindow.get());
            auto *settingsView                 = new SettingsView(std::move(settingsVM), m_mainWindow.get());
            auto *exposureSettingsView         = new ExposureSettingsView(std::move(exposureSettingsVM), m_mainWindow.get());
            auto *favoriteExposureSettingsView = new FavoriteExposureSettingsView(std::move(favoriteExposureSettingsVM), m_mainWindow.get());
            auto *recapExposureSettingsView    = new RecapExposureSettingsView(std::move(recapExposureSettingsVM), m_mainWindow.get());
            auto *progressExposureView         = new ProgressExposureView(std::move(progressExposureVM), m_mainWindow.get());
            auto *completeExposureView         = new CompleteExposureView(std::move(completeExposureVM), m_mainWindow.get());
            auto *saveExposureSettingsView     = new SaveExposureSettingsView(std::move(saveExposureSettingsVM), m_mainWindow.get());
            auto *versionView                  = new VersionView(std::move(versionVM), m_mainWindow.get());
            auto *temperatureView              = new TemperatureView(std::move(temperatureVM), m_mainWindow.get());
            auto *operatingTimeView            = new OperatingTimesView(std::move(operatingTimeVM), m_mainWindow.get());
            auto *screenshotView               = new ScreenshotExportView(std::move(screenshotVM), m_mainWindow.get());
            auto *ledTestView                  = new LedTestView(std::move(ledTestVM), m_mainWindow.get());
            auto *updateSoftwareView           = new UpdateSoftwareView(std::move(updateSoftwareVM), m_mainWindow.get());
            auto *distanceView                 = new DistanceView(std::move(distanceVM), m_mainWindow.get());
            auto *visualisationView            = new VisualisationView(std::move(visualisationVM), m_mainWindow.get());
            auto *loadParametersView           = new LoadParametersView(std::move(loadParametersVM), m_mainWindow.get());
            auto *saveParametersView           = new SaveParametersView(std::move(saveParametersVM), m_mainWindow.get());

            m_mainWindow->addView(Kub3::UI::ViewId::HOME_VIEW, homeView);
            m_mainWindow->addView(Kub3::UI::ViewId::HOME_EIGHT_VIEW, homeEightView);
            m_mainWindow->addView(Kub3::UI::ViewId::MACHINE_STATUS_VIEW, machineStatusView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_VIEW, settingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::EXPOSURE_SETTINGS_VIEW, exposureSettingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::FAVORITE_EXPOSURE_SETTINGS_VIEW, favoriteExposureSettingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::RECAP_EXPOSURE_SETTINGS_VIEW, recapExposureSettingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::PROGRESS_EXPOSURE_VIEW, progressExposureView);
            m_mainWindow->addView(Kub3::UI::ViewId::COMPLETE_EXPOSURE_VIEW, completeExposureView);
            m_mainWindow->addView(Kub3::UI::ViewId::SAVE_EXPOSURE_SETTINGS_VIEW, saveExposureSettingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_ABOUT_VIEW, versionView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_TEMPERATURE_VIEW, temperatureView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_OPERATING_TIMES_VIEW, operatingTimeView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_SCREENSHOT_EXPORT_VIEW, screenshotView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_LED_TEST_VIEW, ledTestView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_UPDATE_SOFTWARE_VIEW, updateSoftwareView);
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_DISTANCE_VIEW, distanceView);
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_VISUALISATION_VIEW, visualisationView);
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_LOAD_PARAMETERS_VIEW, loadParametersView);
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_SAVE_PARAMETERS_VIEW, saveParametersView);
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
        // --- MachineStatusViewModel
        auto *msvm = m_machineStatusVM.get();
        QObject::connect(m_mainWindow.get(), &MainWindow::s_initializationRequest, m_masterFSM, &MFSM::MasterFSM::ps_requestInitialization);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_exposureSliderValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestExposureUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_gainSliderValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestGainUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_framerateValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestFrameRateUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_centeredZoomValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestCenteredZoomUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_roiChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestROIUpdate);

        // 3. Logic -> UI Wiring
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_stateChanged, m_mainWindow.get(), &MainWindow::ps_stateChanged);
        msvm->bindConnection(m_hwManager.get(), &HAL::HardwareManager::s_cameraFrameReady,
                             msvm, &VM::BaseVisionViewModel::ps_onCameraFrameReceived);
        msvm->bindConnection(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_machineValueChanged,
                             msvm, &VM::MachineStatusViewModel::ps_handleSensorValueChanged);
        m_visualisationVM->bindConnection(m_hwManager.get(), &HAL::HardwareManager::s_cameraFrameReady,
                                          m_visualisationVM.get(), &VM::BaseVisionViewModel::ps_onCameraFrameReceived);

        return *this;
    }

    int ApplicationBuilder::run(QApplication &app)
    {
        qInfo() << "[ApplicationBuilder::run]: Starting Hardware Manager.";
        m_hwManager->startAll();

        qInfo() << "[ApplicationBuilder::run]: Starting Logic Thread.";
        m_logicThread->start();

        qInfo() << "[ApplicationBuilder::run]: Showing UI.";
        m_mainWindow->ps_openView(Kub3::UI::ViewId::HOME_EIGHT_VIEW);
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
