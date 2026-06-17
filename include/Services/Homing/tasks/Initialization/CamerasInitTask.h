#pragma once

#include <Config/conf.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    typedef struct initialization_camera_motor_bundle_s {
        Shared<HAL::Act::IPositionMotor> motor;
        Config::kinematic_profile_t kinematicProfile;
    } init_cam_bundle_t;

    class CamerasInitTask final : public ITask
    {
    public:
        CamerasInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                        const Config::process_config_t &processConfig,
                        init_cam_bundle_t leftCamXMotor,
                        init_cam_bundle_t leftCamYMotor,
                        init_cam_bundle_t rightCamXMotor,
                        init_cam_bundle_t rightCamYMotor);

        void start(void) override;
        bool tick(void) override;

    private:
        void stopMotorIfMoving(Shared<HAL::Act::IPositionMotor> motor);
        void handleSingleMotorLogic(const init_cam_bundle_t &bundle, bool limitValue);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        const Config::process_config_t &m_processConf;

        init_cam_bundle_t m_leftCamXBundle;
        init_cam_bundle_t m_leftCamYBundle;
        init_cam_bundle_t m_rightCamXBundle;
        init_cam_bundle_t m_rightCamYBundle;
    };

}
