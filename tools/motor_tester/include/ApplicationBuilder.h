#pragma once

#include <QApplication>
#include <QThread>

#include "./MotorTestController.h"
#include "./MotorTestViewModel.h"
#include "./MotorTestWindow.h"
#include <Config/ConfigLoader.h>
#include <HAL/HardwareManager.h>
#include <HAL/MachineStatus/MachineStatusRepo.h>
#include <utils.h>

namespace Kub3::Tools::MotorTester
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
        QThread *m_logicThread            = nullptr;
        MotorTestController *m_controller = nullptr;

        // Tier 1
        Unique<MotorTestWindow> m_mainWindow;
        Shared<MotorTestViewModel> m_viewModel;
    };

}
