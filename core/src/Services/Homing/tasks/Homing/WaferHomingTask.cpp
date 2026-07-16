#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Homing/WaferHomingTask.h>

namespace Kub3::Services
{

    WaferHomingTask::WaferHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                     Shared<HAL::Act::IPositionMotor> motor,
                                     Config::kinematic_profile_t fastProfile,
                                     Config::kinematic_profile_t fineProfile) :
        m_repo(std::move(repo)),
        m_motor(std::move(motor)),
        m_fastProfile(std::move(fastProfile)),
        m_fineProfile(std::move(fineProfile))
    {
    }

    void WaferHomingTask::start(void)
    {
        const bool cw1 = HAL::MS::readBool(m_repo, CW1);
        const bool cw2 = HAL::MS::readBool(m_repo, CW2);

        if (cw2)
        {
            m_step = Step::Finished;
            return;
        }

        // Checks if the current position is already beyond the deceleration threshold
        if (cw1)
        {
            m_step = Step::SlowApproach;
            m_motor->moveDirection(HAL::Act::MotorDirection::Negative, m_fineProfile);
        }
        else
        {
            m_step = Step::FastApproach;
            m_motor->moveDirection(HAL::Act::MotorDirection::Negative, m_fastProfile);
        }
    }

    bool WaferHomingTask::tick(void)
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

        if (m_step == Step::FastApproach)
        {
            const bool cw1 = HAL::MS::readBool(m_repo, CW1);

            if (cw1)
            {
                m_step = Step::SlowApproach;
                m_motor->moveDirection(HAL::Act::MotorDirection::Negative, m_fineProfile);
            }
        }

        return false;
    }

}
