#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/ZMotorsReachLimitTask.h>

namespace Kub3::Services
{

    ZMotorsReachLimitTask::ZMotorsReachLimitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                                 z_motor_bundle_t leftMotorBundle,
                                                 z_motor_bundle_t rightMotorBundle,
                                                 z_motor_bundle_t backMotorBundle,
                                                 const char *limitId,
                                                 bool limitTargetState) :
        m_repo(std::move(repo)),
        m_leftMotorBundle(std::move(leftMotorBundle)),
        m_rightMotorBundle(std::move(rightMotorBundle)),
        m_backMotorBundle(std::move(backMotorBundle)),
        m_limitId(limitId),
        m_limitTargetState(limitTargetState) {}

    void ZMotorsReachLimitTask::start(void)
    {
        const bool currentLimitState = HAL::MS::readBool(m_repo, m_limitId);

        if (currentLimitState != m_limitTargetState)
            moveAll();
    }

    bool ZMotorsReachLimitTask::tick(void)
    {
        const bool currentLimitState = HAL::MS::readBool(m_repo, m_limitId);

        if (currentLimitState != m_limitTargetState)
        {
            moveAll();
            return false;
        }
        stopAll();
        return true;
    }

    void ZMotorsReachLimitTask::moveAll(void)
    {
        const bool z1                 = HAL::MS::readBool(m_repo, Z1);
        const bool waferOn            = HAL::MS::readBool(m_repo, WAFER_ON);
        const bool z2                 = HAL::MS::readBool(m_repo, Z2);
        const bool fineMotionRequired = z1 && waferOn && !z2;
        const auto direction          = m_limitTargetState ? HAL::Act::MotorDirection::Positive : HAL::Act::MotorDirection::Negative;

        if (fineMotionRequired)
        {
            m_leftMotorBundle.motor->moveDirection(direction, m_leftMotorBundle.fineProfile);
            m_rightMotorBundle.motor->moveDirection(direction, m_rightMotorBundle.fineProfile);
            m_backMotorBundle.motor->moveDirection(direction, m_backMotorBundle.fineProfile);
        }
        else
        {
            m_leftMotorBundle.motor->moveDirection(direction, m_leftMotorBundle.fastProfile);
            m_rightMotorBundle.motor->moveDirection(direction, m_rightMotorBundle.fastProfile);
            m_backMotorBundle.motor->moveDirection(direction, m_backMotorBundle.fastProfile);
        }
    }

    void ZMotorsReachLimitTask::stopAll(void)
    {
        m_leftMotorBundle.motor->emergencyStop();
        m_rightMotorBundle.motor->emergencyStop();
        m_backMotorBundle.motor->emergencyStop();
    }

}
