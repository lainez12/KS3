#pragma once

#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    typedef struct stage_motor_bundle_s {
        Shared<HAL::Act::IPositionMotor> motor;
        Config::kinematic_profile_t kinematic;
        double centerPositionMm;
    } stage_motor_bundle_t;

    class AlignmentStagesHomingTask final : public ITask
    {
    public:
        AlignmentStagesHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                  stage_motor_bundle_t xMotorBundle,
                                  stage_motor_bundle_t yMotorBundle,
                                  stage_motor_bundle_t thetaMotorBundle);

        void start(void) override;
        bool tick(void) override;

    private:
        // Returns true when stage is centered (motor reached center position)
        bool handleSingleMotorLogic(const stage_motor_bundle_t &bundle, const double centerPosMm);
        double readCenterPosition(const stage_motor_bundle_t &bundle, const char *label);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        stage_motor_bundle_t m_xMotorBundle;
        stage_motor_bundle_t m_yMotorBundle;
        stage_motor_bundle_t m_thetaMotorBundle;
    };

}
