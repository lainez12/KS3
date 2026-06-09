#pragma once

#include <array>

#include <Config/kinematics.h>
#include <HAL/Actuators/Motors/IMotor.h>
#include <Services/ITask.h>
#include <utils.h>

#include "../IContactService.h"

namespace Kub3::Services
{

    class SaveCurrentPlanTask final : public ITask
    {
    public:
        SaveCurrentPlanTask(std::array<Shared<HAL::Act::IMotor>, 3> motors,
                            Optional<plan_deltas_t> &outPlanDeltas);

        void start(void) override;
        bool tick(void) override;

    private:
        std::array<Shared<HAL::Act::IMotor>, 3> m_motors;
        Optional<plan_deltas_t> &m_outPlanDeltas;
    };

} // namespace Kub3::Services
