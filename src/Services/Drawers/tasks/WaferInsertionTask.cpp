#include "Services/Drawers/tasks/WaferInsertionTask.h"
#include "HAL/MachineStatus/utils.h"

// TODO: load params from config file
#define SLOW_SPEED_MM_S 10
#define FAST_SPEED_MM_S 100

namespace Kub3::Services
{

    WaferInsertionTask::WaferInsertionTask(Shared<HAL::Act::IMotor> motor, Shared<HAL::MS::IMachineStatusRepo> repo) :
        m_motor(std::move(motor)),
        m_repo(std::move(repo))
    {
    }

    void WaferInsertionTask::start(void)
    {
        const bool cw1 = HAL::MS::readBool(m_repo, CW1);
        const bool cw2 = HAL::MS::readBool(m_repo, CW2);

        if (cw2)
        {
            m_step = Step::Finished;
            return;
        }

        m_motor->setTargetSpeed(cw1 ? SLOW_SPEED_MM_S : FAST_SPEED_MM_S);
        m_motor->moveRelative(NEGATIVE_INFINITE);
    }

    bool WaferInsertionTask::tick()
    {
        const bool cw2 = HAL::MS::readBool(m_repo, CW2);

        if (cw2) // EXIT condition
        {
            if (m_step != Step::Finished)
            {
                m_motor->emergencyStop();
                m_step = Step::Finished;
            }
            return true;
        }

        const bool cw1              = HAL::MS::readBool(m_repo, CW1);
        const uint32_t desiredSpeed = cw1 ? SLOW_SPEED_MM_S : FAST_SPEED_MM_S;

        if (m_motor->getTargetSpeed() != desiredSpeed || !m_motor->isMoving())
        {
            m_motor->setTargetSpeed(desiredSpeed);
            m_motor->moveRelative(NEGATIVE_INFINITE);
        }

        return false;
    }

}
