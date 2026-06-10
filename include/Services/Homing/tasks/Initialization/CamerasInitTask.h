#pragma once

#include <Config/conf.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class CamerasInitTask final : public ITask
    {
    public:
        CamerasInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                        const Config::process_config_t &processConfig,
                        Shared<HAL::Act::IPositionMotor> leftCamXMotor,
                        Shared<HAL::Act::IPositionMotor> leftCamYMotor,
                        Shared<HAL::Act::IPositionMotor> rightCamXMotor,
                        Shared<HAL::Act::IPositionMotor> rightCamYMotor,
                        Config::kinematic_profile_t kinematicProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        void stopMotorIfMoving(Shared<HAL::Act::IPositionMotor> motor);
        void handleSingleMotorLogic(Shared<HAL::Act::IPositionMotor> motor, bool limitValue);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        const Config::process_config_t &m_processConf;

        Shared<HAL::Act::IPositionMotor> m_leftCamXMotor;
        Shared<HAL::Act::IPositionMotor> m_leftCamYMotor;
        Shared<HAL::Act::IPositionMotor> m_rightCamXMotor;
        Shared<HAL::Act::IPositionMotor> m_rightCamYMotor;
        Config::kinematic_profile_t m_kinematicProfile;
    };

}
