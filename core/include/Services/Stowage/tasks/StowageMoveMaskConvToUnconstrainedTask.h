#pragma once

#include <Config/kinematics.h>
#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

#include "./structs.h"

namespace Kub3::Services
{

    class StowageMoveMaskConvToUnconstrainedTask : public ITask
    {
    public:
        StowageMoveMaskConvToUnconstrainedTask(
            Shared<HAL::MS::IMachineStatusRepo> repo,
            const stowage_mask_motor_bundle_t &bundle);

        void start(void) override;
        bool tick(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        const stowage_mask_motor_bundle_t m_motorBundle;
        double m_targetPosMm = 0.0;
    };

}