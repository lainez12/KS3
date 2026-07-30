#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Stowage/tasks/StowageEnableMaskVacuumWhenNeededTask.h>

namespace Kub3::Services
{

    StowageEnableMaskVacuumWhenNeededTask::StowageEnableMaskVacuumWhenNeededTask(
        Shared<HAL::MS::IMachineStatusRepo> repo,
        Shared<HAL::Act::IValve> valve,
        const stowage_mask_motor_bundle_t &bundle) :
        m_repo(std::move(repo)),
        m_valve(std::move(valve)),
        m_bundle(bundle),
        m_isMovingNegative(false)
    {
    }

    void StowageEnableMaskVacuumWhenNeededTask::start(void)
    {
        m_lastPosition.reset();
        m_lastArdkoCount.reset();
        m_isMovingNegative = false;
    }

    bool StowageEnableMaskVacuumWhenNeededTask::tick(void)
    {
        if (HAL::MS::readBool(m_repo, MASK_VACUUM_ACTIVE)) // Suction detected
        {
            postSuccess("The mask holder is secured by vacuum.");
            return true;
        }

        if (!m_bundle.motor)
        {
            abort("Null pointer for motor in StowageEnableMaskVacuumWhenNeededTask");
            return true;
        }
        if (!m_valve)
        {
            abort("Null pointer for valve in StowageEnableMaskVacuumWhenNeededTask");
            return true;
        }

        const double currentPosition    = m_bundle.motor->getEncoderPositionMm();
        const uint8_t currentArdkoCount = currentActiveArdkoCount();

        // If this is the first tick, just initialize our tracking variables and wait for next tick
        if (!m_lastPosition.has_value() || !m_lastArdkoCount.has_value())
        {
            m_lastPosition   = currentPosition;
            m_lastArdkoCount = currentArdkoCount;
            return false;
        }

        // Determine direction (using motor precision as epsilon to filter out noise)
        const double delta   = currentPosition - m_lastPosition.value();
        const double epsilon = m_bundle.motor->getPrecisionMm(m_bundle.kinematics);

        if (delta > epsilon)
        {
            m_isMovingNegative = false;
        }
        else if (delta < -epsilon)
        {
            m_isMovingNegative = true;
        }
        // (If absolute delta < epsilon, we maintain the previous m_isMovingNegative state)

        // Evaluate condition only if we are actively moving in the negative direction
        if (m_isMovingNegative)
        {
            // Transition from 0 to 1 active ardko
            if (m_lastArdkoCount.value() == 0 && currentArdkoCount >= 1)
            {
                m_valve->open();
                postSuccess("Enabling mask holder vacuum.");
            }
        }

        // Update history for the next tick
        m_lastPosition   = currentPosition;
        m_lastArdkoCount = currentArdkoCount;

        return false; // background task: never ending
    }

    uint8_t StowageEnableMaskVacuumWhenNeededTask::currentActiveArdkoCount() const
    {
        return HAL::MS::readBool(m_repo, ARDKO_BACK_LEFT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_BACK_RIGHT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_FRONT_LEFT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_FRONT_RIGHT_LIMIT);
    }

} // namespace Kub3::Services
