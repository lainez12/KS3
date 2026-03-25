#include "Services/Drawers/tasks/MaskEjectionTask.h"
#include "HAL/MachineStatus/utils.h"

// TODO: load params from config file
#define FAST_SPEED_MM_S 100

namespace Kub3::Services
{

    MaskEjectionTask::MaskEjectionTask(Shared<HAL::Act::IMotor> motor, Shared<HAL::MS::IMachineStatusRepo> repo) :
        m_motor(std::move(motor)),
        m_repo(std::move(repo))
    {
    }

    void MaskEjectionTask::start(void)
    {
        const bool cm0 = HAL::MS::readBool(m_repo, CM0);

        if (cm0)
        {
            m_step = Step::Finished;
            return;
        }

        m_step = Step::FastApproach;
        m_motor->setTargetSpeed(FAST_SPEED_MM_S);
        m_motor->moveRelative(POSITIVE_INFINITE);
    }

    bool MaskEjectionTask::tick(void)
    {
        // Snapshot limit state
        const bool cm0 = HAL::MS::readBool(m_repo, CM0);

        if (cm0) // EXIT condition
        {
            if (m_step != Step::Finished)
            {
                m_motor->emergencyStop();
                m_step = Step::Finished;
            }
            return true;
        }

        // TODO: code this:
        // - Define target speed based on encoder position
        // --- Set `desiredSpeed` to the correct value
        const uint32_t desiredSpeed = 0;

        if (m_motor->getTargetSpeed() != desiredSpeed || !m_motor->isMoving())
        {
            m_motor->setTargetSpeed(desiredSpeed);
            m_motor->moveRelative(POSITIVE_INFINITE);
        }

        return false;
    }

}
