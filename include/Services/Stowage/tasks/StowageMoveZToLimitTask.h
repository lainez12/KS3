#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    typedef struct z_motors_bundle_s {
        Shared<HAL::Act::IMotor> leftMotor;
        Shared<HAL::Act::IMotor> rightMotor;
        Shared<HAL::Act::IMotor> backMotor;
        Config::kinematic_profile_t fastProfile;
        Config::kinematic_profile_t fineProfile;
    } z_motors_bundle_t;

    enum ZLimit
    {
        _Z1       = 0x0,
        _WAFER_ON = 0x1,
        _Z2       = 0x2
    };

    class StowageMoveZToLimitTask : public ITask
    {
    public:
        StowageMoveZToLimitTask(
            Shared<HAL::MS::IMachineStatusRepo> repo,
            z_motors_bundle_t bundle,
            ZLimit targetLimit,
            bool limitTargetState);

        void start(void) override;
        bool tick(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        z_motors_bundle_t m_bundle;
        const ZLimit m_targetLimit;
        const bool m_limitTargetState;

        // Internal State tracking
        HAL::Act::MotorDirection m_direction;
        bool m_started           = false;
        bool m_fineProfileActive = false;
    };

}