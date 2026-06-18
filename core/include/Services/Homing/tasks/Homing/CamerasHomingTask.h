#pragma once

#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    typedef struct homing_camera_motor_bundle_s {
        Shared<HAL::Act::IPositionMotor> motor;
        double centerPositionMm;
        Config::kinematic_profile_t kinematicProfile;
    } homing_cam_bundle_t;

    class CamerasHomingTask final : public ITask
    {
    public:
        CamerasHomingTask(
            Shared<HAL::MS::IMachineStatusRepo> repo,
            homing_cam_bundle_t leftCamXBundle,
            homing_cam_bundle_t leftCamYBundle,
            homing_cam_bundle_t rightCamXBundle,
            homing_cam_bundle_t rightCamYBundle);

        void start(void) override;
        bool tick(void) override;

    private:
        bool handleSingleMotorLogic(const homing_cam_bundle_t &bundle);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        homing_cam_bundle_t m_leftCamXBundle;
        homing_cam_bundle_t m_leftCamYBundle;
        homing_cam_bundle_t m_rightCamXBundle;
        homing_cam_bundle_t m_rightCamYBundle;
    };

}
