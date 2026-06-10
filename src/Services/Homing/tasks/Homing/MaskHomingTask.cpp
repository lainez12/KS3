#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Homing/MaskHomingTask.h>

namespace Kub3::Services
{

    MaskHomingTask::MaskHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                   Shared<HAL::Act::IPositionMotor> motor,
                                   Config::kinematic_profile_t fastProfile,
                                   Config::kinematic_profile_t fineProfile,
                                   Config::kinematic_profile_t contactProfile) :
        m_repo(std::move(repo)),
        m_motor(std::move(motor)),
        m_fastProfile(std::move(fastProfile)),
        m_fineProfile(std::move(fineProfile)),
        m_contactProfile(std::move(contactProfile)) {}

    void MaskHomingTask::start(void)
    {
        const bool cm3 = HAL::MS::readBool(m_repo, CM3);

        if (cm3)
        {
            if (m_motor->isMoving())
                m_motor->emergencyStop();
            m_step = Step::Finished;
            return;
        }

        const bool cm0 = HAL::MS::readBool(m_repo, CM0);
        const bool cm1 = HAL::MS::readBool(m_repo, CM1);
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);

        if (cm2)
        {
            m_step = Step::ReverseClearance;
            m_motor->moveDirection(HAL::Act::MotorDirection::Positive, m_contactProfile);
        }
        else if (cm1)
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

    bool MaskHomingTask::tick(void)
    {
        switch (m_step)
        {
        case Step::FastApproach:
        {
            this->fastApproachLogic();
            break;
        }
        case Step::SlowApproach:
        {
            this->slowApproachLogic();
            break;
        }
        case Step::ReverseClearance:
        {
            this->reverseClearanceLogic();
            break;
        }
        case Step::ContactMode:
        {
            this->contactModeLogic();
            break;
        }
        default:
            break;
        }

        return m_step == Step::Finished;
    }

    // Sub-tick logic functions

    void MaskHomingTask::fastApproachLogic(void)
    {
        const bool cm1 = HAL::MS::readBool(m_repo, CM1);
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);
        const bool cm3 = HAL::MS::readBool(m_repo, CM3);

        if (cm3)
        {
            m_motor->emergencyStop();
            m_step = Step::Finished;
        }
        else if (cm2)
        {
            m_motor->moveDirection(HAL::Act::MotorDirection::Positive, m_contactProfile);
            m_step = Step::ReverseClearance;
        }
        else if (cm1)
        {
            m_motor->moveDirection(HAL::Act::MotorDirection::Negative, m_fineProfile);
            m_step = Step::SlowApproach;
        }
    }

    void MaskHomingTask::slowApproachLogic(void)
    {
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);
        const bool cm3 = HAL::MS::readBool(m_repo, CM3);

        if (cm3)
        {
            m_motor->emergencyStop();
            m_step = Step::Finished;
        }
        else if (cm2)
        {
            m_motor->moveDirection(HAL::Act::MotorDirection::Positive, m_contactProfile);
            m_step = Step::ReverseClearance;
        }
    }

    void MaskHomingTask::reverseClearanceLogic(void)
    {
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);

        if (!cm2)
        {
            m_motor->moveDirection(HAL::Act::MotorDirection::Negative, m_contactProfile);
            m_step = Step::ContactMode;
        }
    }

    void MaskHomingTask::contactModeLogic(void)
    {
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);

        if (cm2)
        {
            m_motor->emergencyStop();
            m_step = Step::Finished;
        }
    }

}
