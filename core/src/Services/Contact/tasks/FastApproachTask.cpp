#include <QDebug>
#include <QString>

#include <Config/helper.h>
#include <Services/Contact/tasks/FastApproachTask.h>

namespace Kub3::Services
{

    FastApproachTask::FastApproachTask(std::array<Shared<HAL::Act::IPositionMotor>, 3> motors,
                                       std::function<double()> maxForceGetter,
                                       double contactThresholdGF,
                                       Config::kinematic_profile_t profile) :
        m_motors(std::move(motors)),
        m_currentMaxForceGetter(std::move(maxForceGetter)),
        m_threshold(contactThresholdGF),
        m_profile(std::move(profile))
    {
    }

    void FastApproachTask::start(void)
    {
        qDebug() << QString("=== [FastApproachTask] Starting with target: %1 gF ===").arg(m_threshold);

        // Check if already beyond threshold
        if (m_currentMaxForceGetter() >= m_threshold)
        {
            m_step = Step::Finished;
            return;
        }

        m_step = Step::Approaching;

        // Command continuous positive movement
        for (auto &motor : m_motors)
        {
            if (motor)
            {
                motor->moveDirection(HAL::Act::MotorDirection::Positive, m_profile);
            }
        }
    }

    bool FastApproachTask::tick(void)
    {
        if (m_step == Step::Finished)
            return true;

        if (m_step == Step::Approaching)
        {
            if (m_currentMaxForceGetter() >= m_threshold)
            {
                qDebug() << QString("=== [FastApproachTask] Contact threshold reached with value: %1 gF ===").arg(m_currentMaxForceGetter());
                for (auto &motor : m_motors)
                {
                    if (motor)
                        motor->emergencyStop();
                }
                m_step = Step::Finished;
                return true;
            }
        }

        return false;
    }

} // namespace Kub3::Services
