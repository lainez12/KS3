#pragma once

#include <HAL/Actuators/Valves/IValve.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class ToggleValveTask final : public ITask
    {
    public:
        explicit ToggleValveTask(
            Shared<HAL::MS::IMachineStatusRepo> repo,
            Shared<HAL::Act::IValve> valve,
            bool targetValveState,
            std::string targetSensor,
            bool targetSensorState);

        void start() override;
        bool tick() override;

    private:
        void toggleValveIfNeeded(void);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Shared<HAL::Act::IValve> m_valve;

        const bool m_targetValveOpenState;
        const std::string m_targetSensor;
        const bool m_targetSensorState = false;
    };

}
