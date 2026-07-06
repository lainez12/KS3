#pragma once

#include <array>

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/Homing/tasks/Homing/ZMotorsHomingTask.h>
#include <Services/Homing/tasks/ZMotorsReachLimitTask.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class ForceSensorsTareTask final : public ITask
    {
    public:
        ForceSensorsTareTask(Shared<HAL::MS::IMachineStatusRepo> repo);

        void start(void) override;
        bool tick(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        uint16_t m_ticksCount = 0;
    };

}
