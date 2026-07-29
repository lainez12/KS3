#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Stowage/tasks/StowageMaskToArdkoCountPositionTask.h>

#define MAX_ARDKO_COUNT 4

namespace Kub3::Services
{

    StowageMaskToArdkoCountPositionTask::StowageMaskToArdkoCountPositionTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                                                             uint8_t targetArdkoCount,
                                                                             const stowage_mask_motor_bundle_t &bundle) :
        m_repo(std::move(repo)),
        m_motorBundle(bundle),
        m_targetArdkoCount(targetArdkoCount > MAX_ARDKO_COUNT ? MAX_ARDKO_COUNT : targetArdkoCount)
    {
    }

    void StowageMaskToArdkoCountPositionTask::start() {}

    bool StowageMaskToArdkoCountPositionTask::tick()
    {
        const uint8_t activeArdkoCount = currentActiveArdkoCount();

        if (activeArdkoCount == m_targetArdkoCount)
        {
            m_motorBundle.motor->emergencyStop();
            return true;
        }

        const bool isMoving                = m_motorBundle.motor->isMoving();
        const HAL::Act::MotorDirection dir = activeArdkoCount < m_targetArdkoCount
                                                 ? HAL::Act::MotorDirection::Negative
                                                 : HAL::Act::MotorDirection::Positive;

        if (!isMoving || m_currDirection != dir)
        {
            if (isMoving)
                m_motorBundle.motor->emergencyStop();
            m_motorBundle.motor->moveDirection(dir, m_motorBundle.kinematics);
            m_currDirection = dir;
        }
        return false;
    }

    uint8_t StowageMaskToArdkoCountPositionTask::currentActiveArdkoCount() const
    {
        return HAL::MS::readBool(m_repo, ARDKO_BACK_LEFT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_BACK_RIGHT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_FRONT_LEFT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_FRONT_RIGHT_LIMIT);
    }

}
