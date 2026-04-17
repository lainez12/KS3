#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Homing/ZMotorsHomingTask.h>

namespace Kub3::Services
{

    ZMotorsHomingTask::ZMotorsHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                         z_motor_bundle_t leftMotorBundle,
                                         z_motor_bundle_t rightMotorBundle,
                                         z_motor_bundle_t backMotorBundle) :
        m_repo(std::move(repo)),
        m_leftMotorBundle(std::move(leftMotorBundle)),
        m_rightMotorBundle(std::move(rightMotorBundle)),
        m_backMotorBundle(std::move(backMotorBundle))
    {}

    void ZMotorsHomingTask::start(void)
    {
    }

    bool ZMotorsHomingTask::tick(void)
    {
        const bool z2                = HAL::MS::readBool(m_repo, Z2);
        const bool waferOn           = HAL::MS::readBool(m_repo, WAFER_ON);
        const bool z1                = HAL::MS::readBool(m_repo, Z1);
        const bool leftLow           = HAL::MS::readBool(m_repo, Z_LEFT_LOW_LIMIT);
        const bool rightLow          = HAL::MS::readBool(m_repo, Z_RIGHT_LOW_LIMIT);
        const bool backLow           = HAL::MS::readBool(m_repo, Z_BACK_LOW_LIMIT);
        const bool fineProfileNeeded = (z1 && waferOn && !z2);
        const bool profileChanged    = (m_fineProfileActive != fineProfileNeeded);

        if (profileChanged)
            m_fineProfileActive = fineProfileNeeded;

        // @note: Calling `_lowerToLimit` ensures motors are all stopped before finishing task
        _lowerToLimit(m_leftMotorBundle, leftLow, fineProfileNeeded, profileChanged);
        _lowerToLimit(m_rightMotorBundle, rightLow, fineProfileNeeded, profileChanged);
        _lowerToLimit(m_backMotorBundle, backLow, fineProfileNeeded, profileChanged);

        return leftLow && rightLow && backLow;
    }

    void ZMotorsHomingTask::_lowerToLimit(const z_motor_bundle_t &bundle,
                                          bool lowLimitReached,
                                          bool fineProfileNeeded,
                                          bool profileChanged)
    {
        if (lowLimitReached)
        {
            if (bundle.motor->isMoving())
                bundle.motor->emergencyStop();
        }
        else
        {
            // Send command if not moving OR if moving but the profile changed
            if (!bundle.motor->isMoving() || profileChanged)
                bundle.motor->moveDirection(
                    HAL::Act::MotorDirection::Negative,
                    fineProfileNeeded ? bundle.fineProfile : bundle.fastProfile);
        }
    }

}
