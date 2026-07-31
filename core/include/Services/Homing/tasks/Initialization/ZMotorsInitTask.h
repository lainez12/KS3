#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/Homing/tasks/Homing/ZMotorsHomingTask.h>
#include <Services/Homing/tasks/ZMotorsReachLimitTask.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class ZMotorsInitTask final : public ITask
    {
    public:
        ZMotorsInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                        z_motor_bundle_t leftMotorBundle,
                        z_motor_bundle_t rightMotorBundle,
                        z_motor_bundle_t backMotorBundle);

        void start(void) override;
        bool tick(void) override;

    private:
        void loweringLogic(void);
        void reverseClearanceLogic(void);
        void contactLogic(void);

        void _lowerToLimit(z_motor_bundle_t const &bundle, bool lowLimitReached, bool fineProfileNeeded, bool profileChanged);
        void _climbOutOfLimit(const z_motor_bundle_t &bundle, bool limitValue);

    private:
        enum class Step
        {
            Lowering,
            ReverseClearance,
            Contact
        };

        Step m_step              = Step::Lowering;
        bool m_fineProfileActive = false;

        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        z_motor_bundle_t m_leftMotorBundle;
        z_motor_bundle_t m_rightMotorBundle;
        z_motor_bundle_t m_backMotorBundle;
    };

}
