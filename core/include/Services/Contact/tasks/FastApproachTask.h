#pragma once

#include <array>
#include <functional>

#include <Config/kinematics.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class FastApproachTask final : public ITask
    {
    public:
        enum class Step
        {
            Init,
            Approaching,
            Finished
        };

        FastApproachTask(std::array<Shared<HAL::Act::IPositionMotor>, 3> motors,
                         std::function<double()> maxForceGetter,
                         double contactThresholdGF,
                         Config::kinematic_profile_t profile);

        void start(void) override;
        bool tick(void) override;

    private:
        std::array<Shared<HAL::Act::IPositionMotor>, 3> m_motors;
        std::function<double()> m_currentMaxForceGetter;
        const double m_threshold;
        Config::kinematic_profile_t m_profile;

        Step m_step = Step::Init;
    };

} // namespace Kub3::Services
