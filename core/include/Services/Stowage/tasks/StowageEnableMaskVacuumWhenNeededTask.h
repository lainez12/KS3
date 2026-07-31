#pragma once

#include <Config/kinematics.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/Actuators/Valves/IValve.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <optional>
#include <utils.h>

#include "./structs.h"

namespace Kub3::Services
{

    class StowageEnableMaskVacuumWhenNeededTask : public ITask
    {
    public:
        StowageEnableMaskVacuumWhenNeededTask(
            Shared<HAL::MS::IMachineStatusRepo> repo,
            Shared<HAL::Act::IValve> valve,
            const stowage_mask_motor_bundle_t &bundle);

        void start(void) override;
        bool tick(void) override;

    private:
        uint8_t currentActiveArdkoCount() const;

        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Shared<HAL::Act::IValve> m_valve;
        stowage_mask_motor_bundle_t m_bundle;

        // State tracking
        std::optional<double> m_lastPosition;
        std::optional<uint8_t> m_lastArdkoCount;
        bool m_isMovingNegative;
    };

} // namespace Kub3::Services