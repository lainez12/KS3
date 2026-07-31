#pragma once

#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    typedef struct z_motor_bundle_s {
        Shared<HAL::Act::IPositionMotor> motor;
        Config::kinematic_profile_t fastProfile;
        Config::kinematic_profile_t fineProfile;
    } z_motor_bundle_t;

    class ZMotorsHomingTask final : public ITask
    {
    public:
        ZMotorsHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                          z_motor_bundle_t leftMotorBundle,
                          z_motor_bundle_t rightMotorBundle,
                          z_motor_bundle_t backMotorBundle,
                          double maxTiltMm);

        void start(void) override;
        bool tick(void) override;

    private:
        void _lowerToLimit(
            const z_motor_bundle_t &bundle,
            bool lowLimitReached,
            bool fineProfileNeeded,
            bool profileChanged,
            bool pauseForLeveling);

    private:
        bool m_fineProfileActive = false;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        const double m_maxTiltMm = 0.5;

        z_motor_bundle_t m_leftMotorBundle;
        z_motor_bundle_t m_rightMotorBundle;
        z_motor_bundle_t m_backMotorBundle;
    };

}
