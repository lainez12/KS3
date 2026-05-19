#include "Services/Drawers/tasks/WaferEjectionTask.h"
#include "HAL/MachineStatus/utils.h"

namespace Kub3::Services
{

    WaferEjectionTask::WaferEjectionTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                         Shared<HAL::Act::IMotor> motor,
                                         Config::kinematic_profile_t fastProfile,
                                         Config::kinematic_profile_t fineProfile,
                                         int32_t finePositionThreshold) :
        m_repo(std::move(repo)),
        m_motor(std::move(motor)),
        m_fastProfile(fastProfile),
        m_fineProfile(fineProfile),
        m_finePositionThreshold(finePositionThreshold)
    {
    }

    void WaferEjectionTask::start(void)
    {
        const bool cw0 = HAL::MS::readBool(m_repo, CW0);

        if (cw0)
        {
            m_step = Step::Finished;
            return;
        }

        const int32_t position = HAL::MS::readInt32(m_repo, WAFER_ENCODER);

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

    bool WaferEjectionTask::tick(void)
    {
        // Snapshot limit state
        const bool cw0 = HAL::MS::readBool(m_repo, CW0);

        if (cw0) // EXIT condition
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
            const int32_t position = HAL::MS::readInt32(m_repo, WAFER_ENCODER);

            if (position >= m_finePositionThreshold)
            {
                m_step = Step::SlowApproach;
                m_motor->moveDirection(HAL::Act::MotorDirection::Positive, m_fineProfile);
            }
        }

        return false;
    }

}
