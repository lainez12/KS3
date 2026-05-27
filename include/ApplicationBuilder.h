#pragma once

#include <QApplication>
#include <QString>
#include <QThread>
#include <memory>
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
#include <ViewModels/HomeViewModel.h>
#include <ViewModels/SettingsViewModel.h>
#include <ViewModels/ViewModelsSettings/MachineStatusViewModel.h>
#include <Views/MainWindow.h>

namespace Kub3 {

    class ApplicationBuilder final {
    public:
        ApplicationBuilder()  = default;
        ~ApplicationBuilder() = default;

        // Builder Interface
        ApplicationBuilder &loadConfigurations(const std::string &hwPath, const std::string &processPath);
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

        // Tier 3: I/O and Data
        Shared<HAL::MS::MachineStatusRepo> m_repo;
        Unique<HAL::HardwareManager> m_hwManager;

        // Tier 2: Logic
        // --- Master Finite State Machine & Thread
        QThread *m_logicThread       = nullptr; // Heap-allocated for QThread ownership
        MFSM::MasterFSM *m_masterFSM = nullptr; // Heap-allocated for QThread ownership
        // --- Services
        Shared<Services::IDrawerService> m_drawerService;
        Shared<Services::IHomingService> m_homingService;

        // Tier 1: UI
        Unique<MainWindow> m_mainWindow;
    };

} // namespace Kub3
