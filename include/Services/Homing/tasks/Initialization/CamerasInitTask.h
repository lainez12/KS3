#pragma once

#include <Config/machine_config.h>
#include <HAL/Actuators/Motors/IMotor.h>
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
                        Shared<HAL::Act::IMotor> leftCamXMotor,
                        Shared<HAL::Act::IMotor> leftCamYMotor,
                        Shared<HAL::Act::IMotor> rightCamXMotor,
                        Shared<HAL::Act::IMotor> rightCamYMotor,
                        Config::kinematic_profile_t kinematicProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        void stopMotorIfMoving(Shared<HAL::Act::IMotor> motor);
        void handleSingleMotorLogic(Shared<HAL::Act::IMotor> motor, bool limitValue);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        const Config::process_config_t &m_processConf;

        Shared<HAL::Act::IMotor> m_leftCamXMotor;
        Shared<HAL::Act::IMotor> m_leftCamYMotor;
        Shared<HAL::Act::IMotor> m_rightCamXMotor;
        Shared<HAL::Act::IMotor> m_rightCamYMotor;
        Config::kinematic_profile_t m_kinematicProfile;
    };

}
