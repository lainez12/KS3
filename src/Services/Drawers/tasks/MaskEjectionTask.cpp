#include "Services/Drawers/tasks/MaskEjectionTask.h"
#include "HAL/MachineStatus/utils.h"

namespace Kub3::Services
{

    MaskEjectionTask::MaskEjectionTask(Shared<HAL::Act::IMotor> motor,
                                       Shared<HAL::MS::IMachineStatusRepo> repo,
                                       Config::kinematic_profile_t fastProfile,
                                       Config::kinematic_profile_t fineProfile,
                                       int32_t finePositionThreshold) :
        m_motor(std::move(motor)),
        m_repo(std::move(repo)),
        m_fastProfile(std::move(fastProfile)),
        m_fineProfile(std::move(fineProfile)),
        m_finePositionThreshold(finePositionThreshold)
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

        const int32_t position = HAL::MS::readInt(m_repo, MASK_ENCODER);

        // Checks if the current position is already beyond the deceleration threshold
        if (position >= m_finePositionThreshold)
        {
            m_step = Step::SlowApproach;
            m_motor->moveDirection(HAL::Act::MotorDirection::Positive, m_fineProfile);
        }
        else
        {
            m_step = Step::FastApproach;
            m_motor->moveDirection(HAL::Act::MotorDirection::Positive, m_fastProfile);
        }
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

        if (m_step == Step::FastApproach)
        {
            const int32_t position = HAL::MS::readInt(m_repo, MASK_ENCODER);

            if (position >= m_finePositionThreshold)
            {
                m_step = Step::SlowApproach;
                m_motor->moveDirection(HAL::Act::MotorDirection::Positive, m_fineProfile);
            }
        }

        return false;
    }

}
