#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    typedef struct camera_motor_bundle_s {
        Shared<HAL::Act::IMotor> motor;
        double centerPositionMm;
    } camera_motor_bundle_t;

    class CamerasHomingTask final : public ITask
    {
    public:
        CamerasHomingTask(
            Shared<HAL::MS::IMachineStatusRepo> repo,
            camera_motor_bundle_t leftCamXBundle,
            camera_motor_bundle_t leftCamYBundle,
            camera_motor_bundle_t rightCamXBundle,
            camera_motor_bundle_t rightCamYBundle,
            Config::kinematic_profile_t kinematicProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        bool handleSingleMotorLogic(const camera_motor_bundle_t &bundle);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        camera_motor_bundle_t m_leftCamXBundle;
        camera_motor_bundle_t m_leftCamYBundle;
        camera_motor_bundle_t m_rightCamXBundle;
        camera_motor_bundle_t m_rightCamYBundle;
        Config::kinematic_profile_t m_kinematicProfile;
    };

}
