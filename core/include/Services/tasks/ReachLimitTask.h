#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class ReachLimitTask final : public ITask
    {
    public:
        ReachLimitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                       Shared<HAL::Act::IMotor> motor,
                       Config::kinematic_profile_t kinematic,
                       HAL::Act::MotorDirection direction,
                       const char *limitId,
                       bool limitTargetState);

        void start(void) override;
        bool tick(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Shared<HAL::Act::IMotor> m_motor;
        Config::kinematic_profile_t m_kinematic;
        HAL::Act::MotorDirection m_direction;
        const char *m_limitId;
        const bool m_limitTargetState;
    };

}
