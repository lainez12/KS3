#pragma once

#include <QApplication>
#include <QPointer>
#include <QString>
#include <QThread>
#include <string>

// Configuration & version
#include <Config/ConfigLoader.h>
#include <version.h>
// --- Hardware manager
#include <HAL/HardwareManager.h>
// --- Machine status repository
#include <HAL/MachineStatus/MachineStatusRepo.h>
// --- Master Finite state Machine
#include <MFSM/MasterFSM.h>
// --- UI
#include <ViewModels/Exposure/SaveExposureSettingsViewModel.h>
#include <ViewModels/ExposureMenuViewModel.h>
#include <ViewModels/ExposureModeViewModel.h>
#include <ViewModels/HomeViewModel.h>
#include <ViewModels/Settings/MachineStatusViewModel.h>
#include <ViewModels/SettingsViewModel.h>
#include <Views/MainWindow.h>

namespace Kub3
{

    class ApplicationBuilder final
    {
    public:
        ApplicationBuilder()  = default;
        ~ApplicationBuilder() = default;

        // Builder Interface
        ApplicationBuilder &loadConfigurations(const std::string &hwPath,
                                               const std::string &processPath,
                                               const std::string &adminPath);
        ApplicationBuilder &buildHardwareTier(void);
        ApplicationBuilder &buildLogicTier(void);
        ApplicationBuilder &buildUserInterfaceTier(void);
        ApplicationBuilder &wireArchitecture(void);

        // Runs the application
        int run(QApplication &app);

    private slots:
        void powerOff(void);

    private:
        // Configurations
        Config::hardware_config_t m_hwConfig;
        Config::process_config_t m_processConfig;
        Config::admin_config_t m_adminConfig;

        // Tier 3: I/O and Data
        Shared<HAL::MS::MachineStatusRepo> m_repo;
        Unique<HAL::HardwareManager> m_hwManager;

        // Tier 2: Logic
        // --- Master Finite State Machine & Thread
        QThread *m_logicThread       = nullptr; // Heap-allocated for QThread ownership
        MFSM::MasterFSM *m_masterFSM = nullptr; // Heap-allocated for QThread ownership
        // --- Services
        Shared<Services::IHomingService> m_homingService;
        Shared<Services::IDrawerService> m_drawerService;
        Shared<Services::IStowageService> m_stowageService;
        Shared<Services::IAlignmentService> m_alignmentService;
        Shared<Services::IVisionService> m_visionService;
        Shared<Services::IContactService> m_contactService;
        Shared<Services::IExposureService> m_exposureService;

        // Tier 1: UI
        // --- Windows
        Unique<MainWindow> m_mainWindow;
        // --- View models
        QPointer<UI::ViewModels::MachineStatusViewModel> m_machineStatusVM;
        QPointer<UI::ViewModels::ExposureModeViewModel> m_exposureModeVM;
        QPointer<UI::ViewModels::HomeViewModel> m_homeVM;
        QPointer<UI::ViewModels::SettingsViewModel> m_settingsVM;
        QPointer<UI::ViewModels::ExposureMenuViewModel> m_exposureMenuVM;
        QPointer<UI::ViewModels::Exposure::SaveExposureSettingsViewModel> m_saveExposureSettingsVM;
        QPointer<UI::ViewModels::Exposure::ExposureSettingsViewModel> m_exposureSettingsVM;
        QPointer<UI::ViewModels::Exposure::FavoriteExposureSettingsViewModel> m_favoriteExposureSettingsVM;
        QPointer<UI::ViewModels::Exposure::RecapExposureSettingsViewModel> m_recapExposureSettingsVM;
        QPointer<UI::ViewModels::Exposure::ProgressExposureViewModel> m_progressExposureVM;
        QPointer<UI::ViewModels::Exposure::CompleteExposureViewModel> m_completeExposureVM;
        QPointer<UI::ViewModels::Settings::AdminPasswordViewModel> m_configuratorPasswdVM;
        QPointer<UI::ViewModels::Settings::VersionViewModel> m_versionVM;
        QPointer<UI::ViewModels::Settings::TemperatureViewModel> m_temperatureVM;
        QPointer<UI::ViewModels::Settings::OperatingTimesViewModel> m_operatingTimeVM;
        QPointer<UI::ViewModels::Settings::ScreenshotExportViewModel> m_screenshotVM;
        QPointer<UI::ViewModels::Settings::LedTestViewModel> m_ledTestVM;
        QPointer<UI::ViewModels::Settings::UpdateSoftwareViewModel> m_updateSoftwareVM;
        QPointer<UI::ViewModels::Alignment::DistanceViewModel> m_distanceVM;
        QPointer<UI::ViewModels::Alignment::ContactSelectionViewModel> m_contactSelectionVM;
        QPointer<UI::ViewModels::Alignment::VisualisationViewModel> m_visualisationVM;
        QPointer<UI::ViewModels::Alignment::LoadParametersViewModel> m_loadParametersVM;
        QPointer<UI::ViewModels::Alignment::SaveParametersViewModel> m_saveParametersVM;
    };

} // namespace Kub3
