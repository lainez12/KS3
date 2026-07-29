#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <Services/Stowage/tasks/StowageRecordMaskPositionsTask.h>

namespace Kub3::Services
{

    StowageRecordMaskPositionsTask::StowageRecordMaskPositionsTask(
        Shared<HAL::MS::IMachineStatusRepo> repo,
        const stowage_mask_motor_bundle_t &bundle) :
        m_repo(std::move(repo)),
        m_bundle(bundle),
        m_isMovingNegative(false)
    {
    }

    void StowageRecordMaskPositionsTask::start(void)
    {
        m_lastPosition.reset();
        m_lastArdkoCount.reset();
        m_isMovingNegative = false;

        postInfo("Watching for positions to record.");
    }

    bool StowageRecordMaskPositionsTask::tick(void)
    {
        if (!m_bundle.motor)
        {
            abort("Null pointer for motor in StowageRecordMaskPositionsTask");
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

        // Determine direction (using an motor precision as epsilon to filter out noise)
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

        // Evaluate conditions only if we are actively moving in the negative direction (towards ardkos switches activation)
        if (m_isMovingNegative)
        {
            // Transition from 0 to 1 active ardko
            if (m_lastArdkoCount.value() == 0 && currentArdkoCount >= 1)
            {
                postSuccess("Mask conveyor unconstrained position recorded.");
                m_repo->setValueRaw(V_MASK_CONV_UNCONSTRAINED_POS_MM, currentPosition);
            }

            // Transition reaching 4 active ardkos
            if (m_lastArdkoCount.value() < 4 && currentArdkoCount == 4)
            {
                postSuccess("Mask conveyor reception position recorded.");
                m_repo->setValueRaw(V_MASK_CONV_RECEPTION_POS_MM, currentPosition);
            }
        }

        // Update history for the next tick
        m_lastPosition   = currentPosition;
        m_lastArdkoCount = currentArdkoCount;

        return false; // background task: never ending
    }

    uint8_t StowageRecordMaskPositionsTask::currentActiveArdkoCount() const
    {
        return HAL::MS::readBool(m_repo, ARDKO_BACK_LEFT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_BACK_RIGHT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_FRONT_LEFT_LIMIT) +
               HAL::MS::readBool(m_repo, ARDKO_FRONT_RIGHT_LIMIT);
    }

} // namespace Kub3::Services
