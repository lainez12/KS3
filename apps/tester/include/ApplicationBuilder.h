#pragma once

#include <QApplication>
#include <QThread>

#include <Config/ConfigLoader.h>
#include <Controllers/CameraTestController.h>
#include <Controllers/MotorTestController.h>
#include <Controllers/ProcedureTestController.h>
#include <HAL/HardwareManager.h>
#include <HAL/MachineStatus/MachineStatusRepo.h>
#include <TesterMainWindow.h>
#include <ViewModels/CamerasTestViewModel.h>
#include <ViewModels/MotorTestViewModel.h>
#include <ViewModels/ProcedureTestViewModel.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{

    class ApplicationBuilder final
    {
    public:
        ApplicationBuilder()  = default;
        ~ApplicationBuilder() = default;

        void loadConfigurations(const std::string &hwPath, const std::string &processPath);
        void buildHardwareTier();
        void buildLogicTier();
        void buildUserInterfaceTier();
        void wireArchitecture();
        int run(QApplication &app);

    private:
        Config::hardware_config_t m_hwConfig;
        Config::process_config_t m_processConfig;

        // Tier 3
        Shared<HAL::MS::MachineStatusRepo> m_repo;
        Unique<HAL::HardwareManager> m_hwManager;

        // Tier 2
        QThread *m_logicThread                             = nullptr;
        MotorTestController *m_motorTestController         = nullptr;
        ProcedureTestController *m_procedureTestController = nullptr;
        CameraTestController *m_camerasTestController      = nullptr;

        // Tier 1
        Unique<TesterMainWindow> m_mainWindow;
        Shared<MotorTestViewModel> m_motorTestViewModel;
        Shared<ProcedureTestViewModel> m_procedureTestViewModel;
        Shared<CamerasTestViewModel> m_cameraTestViewModel;
    };

}
