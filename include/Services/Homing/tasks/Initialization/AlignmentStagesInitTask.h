#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class AlignmentStagesInitTask final : public ITask
    {
    public:
        AlignmentStagesInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                Shared<HAL::Act::IMotor> xStageMotor,
                                Shared<HAL::Act::IMotor> yStageMotor,
                                Shared<HAL::Act::IMotor> thetaStageMotor,
                                Config::kinematic_profile_t xStageProfile,
                                Config::kinematic_profile_t yStageProfile,
                                Config::kinematic_profile_t thetaStageProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        void stopMotorIfMoving(Shared<HAL::Act::IMotor> motor);
        void handleSingleMotorLogic(Shared<HAL::Act::IMotor> motor, Config::kinematic_profile_t kineProfile, bool limitValue);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        Shared<HAL::Act::IMotor> m_xStageMotor;
        Shared<HAL::Act::IMotor> m_yStageMotor;
        Shared<HAL::Act::IMotor> m_thetaStageMotor;

        Config::kinematic_profile_t m_xStageProfile;
        Config::kinematic_profile_t m_yStageProfile;
        Config::kinematic_profile_t m_thetaStageProfile;
    };

}
