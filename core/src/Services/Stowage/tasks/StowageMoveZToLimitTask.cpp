#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Stowage/tasks/StowageMoveZToLimitTask.h>

namespace
{

    const char *getLimitKey(Kub3::Services::ZLimit limit)
    {
        switch (limit)
        {
        case Kub3::Services::ZLimit::_Z1:
            return Z1;
        case Kub3::Services::ZLimit::_WAFER_ON:
            return WAFER_ON;
        case Kub3::Services::ZLimit::_Z2:
            return Z2;
        }
        return nullptr;
    }

}

namespace Kub3::Services
{

    StowageMoveZToLimitTask::StowageMoveZToLimitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                                     z_motors_bundle_t bundle,
                                                     ZLimit targetLimit,
                                                     bool limitTargetState) :
        m_repo(std::move(repo)),
        m_bundle(std::move(bundle)),
        m_targetLimit(targetLimit),
        m_limitTargetState(limitTargetState)
    {
        // Infer the movement direction based on the desired limit state.
        // True  -> We want the limit to turn ON  -> We are moving UP (Positive).
        // False -> We want the limit to turn OFF -> We are moving DOWN (Negative).
        m_direction = m_limitTargetState ? HAL::Act::MotorDirection::Positive : HAL::Act::MotorDirection::Negative;
    }

    void StowageMoveZToLimitTask::start(void)
    {
        m_started           = false;
        m_fineProfileActive = false;

        switch (m_targetLimit)
        {
        case ZLimit::_Z2:
        {
            postInfo(m_limitTargetState ? "Moving wafer up to alignment zone..."
                                        : "Moving wafer down to alignment zone...");
            break;
        }
        case ZLimit::_WAFER_ON:
        {
            postInfo(m_limitTargetState ? "Stowing wafer..."
                                        : "Unstowing wafer...");
            break;
        }
        case ZLimit::_Z1:
        {
            postInfo(m_limitTargetState ? "Moving alignment block up to stowing zone..."
                                        : "Moving alignment block down out of stowing zone...");
            break;
        }
        default:
            break;
        }
    }

    bool StowageMoveZToLimitTask::tick(void)
    {
        const char *targetLimitKey = getLimitKey(m_targetLimit);

        // Check if the target condition is successfully met
        if (targetLimitKey && HAL::MS::readBool(m_repo, targetLimitKey) == m_limitTargetState)
        {
            // Stop all motors before finishing the task
            if (m_bundle.leftMotor)
                m_bundle.leftMotor->emergencyStop();
            if (m_bundle.rightMotor)
                m_bundle.rightMotor->emergencyStop();
            if (m_bundle.backMotor)
                m_bundle.backMotor->emergencyStop();
            return true;
        }

        // Dynamic Kinematic Profile Selection (Stowage Zone Detection)
        const bool z1 = HAL::MS::readBool(m_repo, Z1);
        const bool z2 = HAL::MS::readBool(m_repo, Z2);

        // We are in the "Stowage Zone" if Z1 is ON and Z2 is OFF.
        const bool fineProfileNeeded = (z1 && !z2);
        const bool profileChanged    = (m_fineProfileActive != fineProfileNeeded);

        if (profileChanged)
            m_fineProfileActive = fineProfileNeeded;

        // Command the hardware if needed
        if (!m_started || profileChanged)
        {
            const auto &activeProfile = m_fineProfileActive ? m_bundle.fineProfile : m_bundle.fastProfile;

            if (m_bundle.leftMotor)
                m_bundle.leftMotor->moveDirection(m_direction, activeProfile);
            if (m_bundle.rightMotor)
                m_bundle.rightMotor->moveDirection(m_direction, activeProfile);
            if (m_bundle.backMotor)
                m_bundle.backMotor->moveDirection(m_direction, activeProfile);

            m_started = true;
        }

        return false;
    }

} // namespace Kub3::Services