#include <HAL/MachineStatus/utils.h>
#include <Services/tasks/ReachLimitTask.h>

namespace Kub3::Services
{

    ReachLimitTask::ReachLimitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                   Shared<HAL::Act::IMotor> motor,
                                   Config::kinematic_profile_t kinematic,
                                   HAL::Act::MotorDirection direction,
                                   const char *limitId,
                                   bool limitTargetState) :
        m_repo(std::move(repo)),
        m_motor(std::move(motor)),
        m_kinematic(std::move(kinematic)),
        m_direction(direction),
        m_limitId(limitId),
        m_limitTargetState(limitTargetState) {}

    void ReachLimitTask::start(void)
    {
        const bool currentLimitState = HAL::MS::readBool(m_repo, m_limitId);

        if (currentLimitState != m_limitTargetState)
            m_motor->moveDirection(m_direction, m_kinematic);
    }

    bool ReachLimitTask::tick(void)
    {
        const bool currentLimitState  = HAL::MS::readBool(m_repo, m_limitId);
        const bool targetStateReached = (currentLimitState == m_limitTargetState);

        if (targetStateReached)
        {
            if (m_motor->isMoving())
                m_motor->emergencyStop();
        }
        else if (!m_motor->isMoving())
            m_motor->moveDirection(m_direction, m_kinematic);

        return targetStateReached;
    }

}
