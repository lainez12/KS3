#pragma once

#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class AlignmentStagesInitTask final : public ITask
    {
    public:
        AlignmentStagesInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                Shared<HAL::Act::IPositionMotor> xStageMotor,
                                Shared<HAL::Act::IPositionMotor> yStageMotor,
                                Shared<HAL::Act::IPositionMotor> thetaStageMotor,
                                Config::kinematic_profile_t xStageProfile,
                                Config::kinematic_profile_t yStageProfile,
                                Config::kinematic_profile_t thetaStageProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        void stopMotorIfMoving(Shared<HAL::Act::IPositionMotor> motor);
        void handleSingleMotorLogic(Shared<HAL::Act::IPositionMotor> motor, Config::kinematic_profile_t kineProfile, bool limitValue);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        Shared<HAL::Act::IPositionMotor> m_xStageMotor;
        Shared<HAL::Act::IPositionMotor> m_yStageMotor;
        Shared<HAL::Act::IPositionMotor> m_thetaStageMotor;

        Config::kinematic_profile_t m_xStageProfile;
        Config::kinematic_profile_t m_yStageProfile;
        Config::kinematic_profile_t m_thetaStageProfile;
    };

}
