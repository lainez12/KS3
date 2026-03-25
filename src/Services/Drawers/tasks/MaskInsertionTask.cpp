#include "Services/Drawers/tasks/MaskInsertionTask.h"
#include "HAL/MachineStatus/utils.h"

// TODO: load params from config file
#define CONTACT_SPEED_MM_S 1
#define SLOW_SPEED_MM_S    10
#define FAST_SPEED_MM_S    100

namespace Kub3::Services
{

    MaskInsertionTask::MaskInsertionTask(Shared<HAL::Act::IMotor> motor, Shared<HAL::MS::IMachineStatusRepo> repo) :
        m_motor(std::move(motor)),
        m_repo(std::move(repo))
    {
    }

    void MaskInsertionTask::start(void)
    {
        const bool cm3 = HAL::MS::readBool(m_repo, CM3);

        if (cm3)
        {
            if (m_motor->isMoving())
                m_motor->emergencyStop();
            m_step = Step::Finished;
            return;
        }

        const bool cm0            = HAL::MS::readBool(m_repo, CM0);
        const bool cm1            = HAL::MS::readBool(m_repo, CM1);
        const bool cm2            = HAL::MS::readBool(m_repo, CM2);
        uint32_t relativeMovement = NEGATIVE_INFINITE;

        if (cm2)
        {
            m_step = Step::ReverseClearance;
            m_motor->setTargetSpeed(CONTACT_SPEED_MM_S);
            relativeMovement = POSITIVE_INFINITE;
        }
        else if (cm1)
        {
            m_step = Step::SlowApproach;
            m_motor->setTargetSpeed(SLOW_SPEED_MM_S);
        }
        else
        {
            m_step = Step::FastApproach;
            m_motor->setTargetSpeed(FAST_SPEED_MM_S);
        }

        m_motor->moveRelative(relativeMovement);
    }

    bool MaskInsertionTask::tick(void)
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

    void MaskInsertionTask::fastApproachLogic(void)
    {
        const bool cm1 = HAL::MS::readBool(m_repo, CM1);
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);
        const bool cm3 = HAL::MS::readBool(m_repo, CM3);

        if (cm3)
        {
            m_step = Step::Finished;
            m_motor->emergencyStop();
        }
        else if (cm2)
        {
            m_step = Step::ReverseClearance;
            m_motor->setTargetSpeed(CONTACT_SPEED_MM_S);
            m_motor->moveRelative(POSITIVE_INFINITE);
        }
        else if (cm1)
        {
            m_step = Step::SlowApproach;
            m_motor->setTargetSpeed(SLOW_SPEED_MM_S);
            m_motor->moveRelative(NEGATIVE_INFINITE);
        }
    }

    void MaskInsertionTask::slowApproachLogic(void)
    {
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);
        const bool cm3 = HAL::MS::readBool(m_repo, CM3);

        if (cm3)
        {
            m_step = Step::Finished;
            m_motor->emergencyStop();
        }
        else if (cm2)
        {
            m_step = Step::ReverseClearance;
            m_motor->setTargetSpeed(CONTACT_SPEED_MM_S);
            m_motor->moveRelative(POSITIVE_INFINITE);
        }
    }

    void MaskInsertionTask::reverseClearanceLogic(void)
    {
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);

        if (!cm2)
        {
            m_step = Step::ContactMode;
            m_motor->setTargetSpeed(CONTACT_SPEED_MM_S);
            m_motor->moveRelative(NEGATIVE_INFINITE);
        }
    }

    void MaskInsertionTask::contactModeLogic(void)
    {
        const bool cm2 = HAL::MS::readBool(m_repo, CM2);

        if (cm2)
        {
            m_motor->emergencyStop();
            m_step = Step::Finished;
        }
    }

}
