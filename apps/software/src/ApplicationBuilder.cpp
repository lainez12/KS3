#include "ViewModels/ExposureModeViewModel.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include <ApplicationBuilder.h>

#include <Config/helper.h>
#include <system.h>
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

namespace
{

    void launchConfigurator(void)
    {
        QString programPath = KUB3_CONFIGURATOR_BIN_PATH;
        QFileInfo fileInfo(programPath);

        if (fileInfo.isRelative())
        {
            QDir appDir(QCoreApplication::applicationDirPath());
            programPath = appDir.absoluteFilePath(programPath);
        }

        if (!QProcess::startDetached(programPath))
        {
            qCritical() << "Failed to launch the configurator program at:" << programPath;
        }
        else
        {
            qInfo() << "Successfully launched configurator at:" << programPath;
        }
    }

}

namespace Kub3
{
#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
    using ConveyorDrawerService = Services::SingleConveyorDrawerService;
#elif defined(KUB_MODEL_8)
    using ConveyorDrawerService = Services::DualConveyorDrawerService;
#endif

    ApplicationBuilder &ApplicationBuilder::loadConfigurations(const std::string &hwPath, const std::string &processPath, const std::string &adminPath)
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

        // Apply keyboard layout
        {
            std::string xkbLayout = translateLocaleToXkb(m_hwConfig.keyboardLayout.toStdString());

            qInfo() << "Translated \"" << m_hwConfig.keyboardLayout << "\" to XKB layout: \"" << xkbLayout << "\"\n";
            if (applyKeyboardLayout(xkbLayout))
                qInfo() << "Successfully changed layout to: " << xkbLayout << "\n";
            else
                qWarning() << "Failed to apply keyboard layout.\n";
        }

        // Strict Dependency Injection
        m_hwManager = std::make_unique<HAL::HardwareManager>(m_repo, m_hwConfig);

        return *this;
    }

    ApplicationBuilder &ApplicationBuilder::buildLogicTier(void)
    {
        qInfo() << "Building Tier 2 (Logic)...";

        auto actReg = m_hwManager->getActuatorRegistry();

        m_homingService    = std::make_shared<Services::HomingService>(actReg, m_repo, m_processConfig, m_hwConfig);
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
            ASSIGN_VIEW_MODEL(UI::ViewModels::ExposureModeViewModel, exposureModeVM, m_exposureModeVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::HomeViewModel, homeVM, m_homeVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::MachineStatusViewModel, machineStatusVM, m_machineStatusVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::SettingsViewModel, settingsVM, m_settingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::ExposureMenuViewModel, exposureMenuVM, m_exposureMenuVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::ExposureSettingsViewModel, exposureSettingsVM, m_exposureSettingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::FavoriteExposureSettingsViewModel, favoriteExposureSettingsVM, m_favoriteExposureSettingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::RecapExposureSettingsViewModel, recapExposureSettingsVM, m_recapExposureSettingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::ProgressExposureViewModel, progressExposureVM, m_progressExposureVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::CompleteExposureViewModel, completeExposureVM, m_completeExposureVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Exposure::SaveExposureSettingsViewModel, saveExposureSettingsVM, m_saveExposureSettingsVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::AdminPasswordViewModel, configuratorPasswdVM, m_configuratorPasswdVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::VersionViewModel, versionVM, m_versionVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::TemperatureViewModel, temperatureVM, m_temperatureVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::OperatingTimesViewModel, operatingTimeVM, m_operatingTimeVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::ScreenshotExportViewModel, screenshotVM, m_screenshotVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::LedTestViewModel, ledTestVM, m_ledTestVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Settings::UpdateSoftwareViewModel, updateSoftwareVM, m_updateSoftwareVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::DistanceViewModel, distanceVM, m_distanceVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::ContactSelectionViewModel, contactSelectionVM, m_contactSelectionVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::VisualisationViewModel, visualisationVM, m_visualisationVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::LoadParametersViewModel, loadParametersVM, m_loadParametersVM, m_repo);
            ASSIGN_VIEW_MODEL(UI::ViewModels::Alignment::SaveParametersViewModel, saveParametersVM, m_saveParametersVM, m_repo);

            // Views creation
            auto *homeView                     = new HomeView(std::move(homeVM), m_mainWindow.get());
            auto *machineStatusView            = new MachineStatusView(std::move(machineStatusVM), m_mainWindow.get());
            auto *exposureMenuView             = new ExposureMenuView(std::move(exposureMenuVM), m_mainWindow.get());
            auto *settingsView                 = new SettingsView(std::move(settingsVM), m_mainWindow.get());
            auto *exposureSettingsView         = new ExposureSettingsView(std::move(exposureSettingsVM), m_mainWindow.get());
            auto *favoriteExposureSettingsView = new FavoriteExposureSettingsView(std::move(favoriteExposureSettingsVM), m_mainWindow.get());
            auto *recapExposureSettingsView    = new RecapExposureSettingsView(std::move(recapExposureSettingsVM), m_mainWindow.get());
            auto *progressExposureView         = new ProgressExposureView(std::move(progressExposureVM), m_mainWindow.get());
            auto *completeExposureView         = new CompleteExposureView(std::move(completeExposureVM), m_mainWindow.get());
            auto *saveExposureSettingsView     = new SaveExposureSettingsView(std::move(saveExposureSettingsVM), m_mainWindow.get());
            auto *configuratorPasswdView       = new AdminPasswordView(std::move(configuratorPasswdVM), m_mainWindow.get());
            auto *versionView                  = new VersionView(std::move(versionVM), m_mainWindow.get());
            auto *temperatureView              = new TemperatureView(std::move(temperatureVM), m_mainWindow.get());
            auto *operatingTimeView            = new OperatingTimesView(std::move(operatingTimeVM), m_mainWindow.get());
            auto *screenshotView               = new ScreenshotExportView(std::move(screenshotVM), m_mainWindow.get());
            auto *ledTestView                  = new LedTestView(std::move(ledTestVM), m_mainWindow.get());
            auto *updateSoftwareView           = new UpdateSoftwareView(std::move(updateSoftwareVM), m_mainWindow.get());
            auto *distanceView                 = new DistanceView(std::move(distanceVM), m_mainWindow.get());
            auto *contactSelectionView         = new ContactSelectionView(std::move(contactSelectionVM), m_mainWindow.get());
            auto *visualisationView            = new VisualisationView(std::move(visualisationVM), m_mainWindow.get());
            auto *loadParametersView           = new LoadParametersView(std::move(loadParametersVM), m_mainWindow.get());
            auto *saveParametersView           = new SaveParametersView(std::move(saveParametersVM), m_mainWindow.get());
            auto *exposureModeView             = new ExposureModeView(std::move(exposureModeVM), m_mainWindow.get());

            m_mainWindow->addView(Kub3::UI::ViewId::EXPOSURE_MODE_VIEW, exposureModeView);
            m_mainWindow->addView(Kub3::UI::ViewId::HOME_VIEW, homeView);
            m_mainWindow->addView(Kub3::UI::ViewId::EXPOSURE_MENU_VIEW, exposureMenuView);
            m_mainWindow->addView(Kub3::UI::ViewId::MACHINE_STATUS_VIEW, machineStatusView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_VIEW, settingsView);
            m_mainWindow->addView(Kub3::UI::ViewId::SETTINGS_ADMIN_CONFIGURATOR_PASSWD_VIEW, configuratorPasswdView);
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
            m_mainWindow->addView(Kub3::UI::ViewId::ALIGNMENT_CONTACT_SELECTION_VIEW, contactSelectionView);
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
        // --- Main window
        QObject::connect(m_mainWindow.get(), &MainWindow::s_requestRetryBoot, m_masterFSM, &MFSM::MasterFSM::ps_requestRetryBoot);
        QObject::connect(m_mainWindow.get(), &MainWindow::s_requestResetError, m_masterFSM, &MFSM::MasterFSM::ps_requestResetError);
        QObject::connect(m_mainWindow.get(), &MainWindow::s_requestPowerOff, m_masterFSM, &MFSM::MasterFSM::ps_systemPowerOff);
        // --- HomeViewModel
        m_homeVM->bindConnection(m_homeVM.get(), &VM::HomeViewModel::s_cancelOperation, m_masterFSM, &MFSM::MasterFSM::ps_requestAbortOperation);
        m_homeVM->bindConnection(m_homeVM.get(), &VM::HomeViewModel::s_initializationRequest, m_masterFSM, &MFSM::MasterFSM::ps_requestInitialization);
        m_homeVM->bindConnection(m_homeVM.get(), &VM::HomeViewModel::s_cmdRunDrawerOperation, m_masterFSM, &MFSM::MasterFSM::ps_requestOperateDrawer);
        // --- Exposure Menu View Model
        m_exposureMenuVM->bindConnection(m_exposureMenuVM.get(), &VM::ExposureMenuViewModel::s_cmdOperateStowage, m_masterFSM, &MFSM::MasterFSM::ps_requestStowage);
        m_exposureMenuVM->bindConnection(m_exposureMenuVM.get(), &VM::ExposureMenuViewModel::s_cmdStartAutolevel, m_masterFSM, &MFSM::MasterFSM::ps_requestAutolevel);
        m_exposureMenuVM->bindConnection(m_exposureMenuVM.get(), &VM::ExposureMenuViewModel::s_cmdCancelOperation, m_masterFSM, &MFSM::MasterFSM::ps_requestAbortOperation);
        m_exposureMenuVM->bindConnection(m_masterFSM, &MFSM::MasterFSM::s_processMessageBroadcast, m_exposureMenuVM.get(), &VM::ExposureMenuViewModel::ps_onProcessMessageBroadcast);
        // --- VisualisationViewModel
        // QObject::connect(m_visualisationVM.get(), &VM::Alignment::VisualisationViewModel::cmdRunCameraMovement, m_masterFSM, &MFSM::MasterFSM::ps_requestPADCameraMovement);
        QObject::connect(m_visualisationVM.get(), &VM::Alignment::VisualisationViewModel::cmdRunAlignmentStageMovement, m_masterFSM, &MFSM::MasterFSM::ps_requestPADAlignmentStageMovement);
        // --- MachineStatusViewModel
        auto *msvm = m_machineStatusVM.get();
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_exposureSliderValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestExposureUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_gainSliderValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestGainUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_framerateValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestFrameRateUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_centeredZoomValueChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestCenteredZoomUpdate);
        QObject::connect(msvm, &VM::MachineStatusViewModel::s_roiChanged, m_masterFSM, &MFSM::MasterFSM::ps_requestROIUpdate);
        // --- ExposureViewModels
        QObject::connect(m_exposureSettingsVM.get(), &VM::Exposure::ExposureSettingsViewModel::s_requestSaveExposureSettings, m_saveExposureSettingsVM.get(), &VM::Exposure::SaveExposureSettingsViewModel::ps_saveExposureSettings);
        QObject::connect(m_exposureSettingsVM.get(), &VM::Exposure::ExposureSettingsViewModel::s_requestExposureSettingsByForm, m_recapExposureSettingsVM.get(), &VM::Exposure::RecapExposureSettingsViewModel::ps_setExposurePreset);
        QObject::connect(m_favoriteExposureSettingsVM.get(), &VM::Exposure::FavoriteExposureSettingsViewModel::s_exposurePresetLoaded, m_recapExposureSettingsVM.get(), &VM::Exposure::RecapExposureSettingsViewModel::ps_setExposurePreset);
        m_progressExposureVM->bindConnection(m_progressExposureVM.get(), &VM::Exposure::ProgressExposureViewModel::s_launchExposure, m_masterFSM, &MFSM::MasterFSM::ps_requestExposure);
        QObject::connect(m_recapExposureSettingsVM.get(), &VM::Exposure::RecapExposureSettingsViewModel::s_exposurePresetLaunched, m_progressExposureVM.get(), &VM::Exposure::ProgressExposureViewModel::ps_launchExposure);
        // --- Settings
        QObject::connect(m_configuratorPasswdVM.get(), &VM::Settings::AdminPasswordViewModel::s_authenticationSuccess, &launchConfigurator);

        // 3. Logic -> UI Wiring
        // @note: `QObject::connect` connections are permanent while `bindConnection` only activates connections when the view is shown
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_errorOccurred, m_mainWindow.get(), &MainWindow::ps_errorOccurred);
        // --- Home View Model
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_systemStateKindChanged, m_homeVM.get(), &VM::HomeViewModel::ps_onSystemStateChanged);
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_operationalSubstateKindChanged, m_homeVM.get(), &VM::HomeViewModel::ps_onOperationalSubstateKindChanged);
        m_homeVM->bindConnection(m_masterFSM, &MFSM::MasterFSM::s_operationCanceled, m_homeVM.get(), &VM::HomeViewModel::ps_operationEnded);
        m_homeVM->bindConnection(m_masterFSM, &MFSM::MasterFSM::s_errorOccurred, m_homeVM.get(), &VM::HomeViewModel::ps_errorOccurred);
        m_homeVM->bindConnection(m_masterFSM, &MFSM::MasterFSM::s_initializationSuccess, m_homeVM.get(), &VM::HomeViewModel::ps_initializationSuccess);
        m_homeVM->bindConnection(m_masterFSM, &MFSM::MasterFSM::s_serviceOpSuccess, m_homeVM.get(), &VM::HomeViewModel::ps_operationEnded);
        m_homeVM->bindConnection(m_masterFSM, &MFSM::MasterFSM::s_serviceOpError, m_homeVM.get(), &VM::HomeViewModel::ps_operationEnded);
        m_homeVM->bindConnection(m_masterFSM, &MFSM::MasterFSM::s_processMessageBroadcast, m_homeVM.get(), &VM::HomeViewModel::ps_onProcessMessageBroadcast);
        // --- Exposure Menu View Model (Stowage)
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_systemStateKindChanged, m_exposureMenuVM.get(), &VM::ExposureMenuViewModel::ps_onSystemStateChanged);
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_operationalSubstateChanged, m_exposureMenuVM.get(), &VM::ExposureMenuViewModel::ps_onOperationalSubstateChanged);
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_postureChanged, m_exposureMenuVM.get(), &VM::ExposureMenuViewModel::ps_onPostureChanged);
        // --- Exposure Mode View Model
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_systemStateKindChanged, m_exposureModeVM.get(), &VM::ExposureModeViewModel::ps_onSystemStateChanged);
        QObject::connect(m_masterFSM, &MFSM::MasterFSM::s_postureChanged, m_exposureModeVM.get(), &VM::ExposureModeViewModel::ps_onPostureChanged);
        // --- Alignment View Model
        msvm->bindConnection(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_machineValueChanged,
                             m_visualisationVM.get(), &VM::Alignment::VisualisationViewModel::ps_handleSensorValueChanged);
        // --- Machine Status View Model
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
        m_mainWindow->ps_openView(Kub3::UI::ViewId::HOME_VIEW);
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
