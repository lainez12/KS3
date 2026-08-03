#pragma once

#include <array>

#include <Config/kinematics.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

#include "../IContactService.h"

namespace Kub3::Services
{

    class SaveCurrentPlanTask final : public ITask
    {
    public:
        SaveCurrentPlanTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                            std::array<Shared<HAL::Act::IPositionMotor>, 3> motors,
                            Optional<plan_deltas_t> &outPlanDeltas);

        void start(void) override;
        bool tick(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        std::array<Shared<HAL::Act::IPositionMotor>, 3> m_motors;
        Optional<plan_deltas_t> &m_outPlanDeltas;
    };

} // namespace Kub3::Services
