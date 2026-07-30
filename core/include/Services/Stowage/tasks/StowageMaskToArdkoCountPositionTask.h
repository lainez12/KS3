#pragma once

#include <Config/kinematics.h>
#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

#include "./structs.h"

namespace Kub3::Services
{

    class StowageMaskToArdkoCountPositionTask : public ITask
    {
    public:
        StowageMaskToArdkoCountPositionTask(
            Shared<HAL::MS::IMachineStatusRepo> repo,
            uint8_t targetArdkoCount,
            const stowage_mask_motor_bundle_t &bundle);

        void start(void) override;
        bool tick(void) override;

    private:
        uint8_t currentActiveArdkoCount() const;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        const stowage_mask_motor_bundle_t m_motorBundle;
        const uint8_t m_targetArdkoCount = 0;

        HAL::Act::MotorDirection m_currDirection = HAL::Act::MotorDirection::Positive;
    };

}