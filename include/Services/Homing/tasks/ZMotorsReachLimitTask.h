#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/Homing/tasks/Homing/ZMotorsHomingTask.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class ZMotorsReachLimitTask final : public ITask
    {
    public:
        ZMotorsReachLimitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                              z_motor_bundle_t leftMotorBundle,
                              z_motor_bundle_t rightMotorBundle,
                              z_motor_bundle_t backMotorBundle,
                              const char *limitId,
                              bool limitTargetState);

        void start(void) override;
        bool tick(void) override;

    private:
        void moveAll(void);
        void stopAll(void);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        z_motor_bundle_t m_leftMotorBundle;
        z_motor_bundle_t m_rightMotorBundle;
        z_motor_bundle_t m_backMotorBundle;
        const char *m_limitId;
        const bool m_limitTargetState;
    };

}
