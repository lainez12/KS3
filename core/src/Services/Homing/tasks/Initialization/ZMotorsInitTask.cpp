#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Initialization/ZMotorsInitTask.h>

namespace Kub3::Services
{

    ZMotorsInitTask::ZMotorsInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                     z_motor_bundle_t leftMotorBundle,
                                     z_motor_bundle_t rightMotorBundle,
                                     z_motor_bundle_t backMotorBundle) :
        m_repo(std::move(repo)),
        m_leftMotorBundle(std::move(leftMotorBundle)),
        m_rightMotorBundle(std::move(rightMotorBundle)),
        m_backMotorBundle(std::move(backMotorBundle))
    {}

    void ZMotorsInitTask::start(void)
    {
        const bool leftMotorLowLimit  = HAL::MS::readBool(m_repo, Z_LEFT_LOW_LIMIT);
        const bool rightMotorLowLimit = HAL::MS::readBool(m_repo, Z_RIGHT_LOW_LIMIT);
        const bool backMotorLowLimit  = HAL::MS::readBool(m_repo, Z_BACK_LOW_LIMIT);

        if (leftMotorLowLimit && rightMotorLowLimit && backMotorLowLimit)
            m_step = Step::ReverseClearance;
        else
            m_step = Step::Lowering;
    }

    bool ZMotorsInitTask::tick(void)
    {
        const bool leftMotorLowLimit  = HAL::MS::readBool(m_repo, Z_LEFT_LOW_LIMIT);
        const bool rightMotorLowLimit = HAL::MS::readBool(m_repo, Z_RIGHT_LOW_LIMIT);
        const bool backMotorLowLimit  = HAL::MS::readBool(m_repo, Z_BACK_LOW_LIMIT);
        const bool allLimitsReached   = leftMotorLowLimit && rightMotorLowLimit && backMotorLowLimit;

        if (m_step == Step::Contact && allLimitsReached)
        {
            m_leftMotorBundle.motor->emergencyStop();
            m_rightMotorBundle.motor->emergencyStop();
            m_backMotorBundle.motor->emergencyStop();
            m_leftMotorBundle.motor->resetEncoder(0.0);
            m_rightMotorBundle.motor->resetEncoder(0.0);
            m_backMotorBundle.motor->resetEncoder(0.0);
            return true;
        }

        switch (m_step)
        {
        case Step::Lowering:
        {
            this->loweringLogic();
            break;
        }
        case Step::ReverseClearance:
        {
            this->reverseClearanceLogic();
            break;
        }
        case Step::Contact:
        {
            this->contactLogic();
            break;
        }
        }

        return false;
    }

    void ZMotorsInitTask::loweringLogic()
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

        // @note: Calling `_lowerToLimit` ensures motors are all stopped when proceeding to next step
        _lowerToLimit(m_leftMotorBundle, leftLow, fineProfileNeeded, profileChanged);
        _lowerToLimit(m_rightMotorBundle, rightLow, fineProfileNeeded, profileChanged);
        _lowerToLimit(m_backMotorBundle, backLow, fineProfileNeeded, profileChanged);

        if (leftLow && rightLow && backLow)
        {
            m_step = Step::ReverseClearance;
            return;
        }
    }

    void ZMotorsInitTask::reverseClearanceLogic(void)
    {
        const bool leftLow  = HAL::MS::readBool(m_repo, Z_LEFT_LOW_LIMIT);
        const bool rightLow = HAL::MS::readBool(m_repo, Z_RIGHT_LOW_LIMIT);
        const bool backLow  = HAL::MS::readBool(m_repo, Z_BACK_LOW_LIMIT);

        // @note: Calling `_climbOutOfLimit` ensures motors are all stopped when proceeding to next step
        _climbOutOfLimit(m_leftMotorBundle, leftLow);
        _climbOutOfLimit(m_rightMotorBundle, rightLow);
        _climbOutOfLimit(m_backMotorBundle, backLow);

        if (!leftLow && !rightLow && !backLow)
        {
            m_step = Step::Contact;
            return;
        }
    }

    void ZMotorsInitTask::contactLogic(void)
    {
        const bool leftLow  = HAL::MS::readBool(m_repo, Z_LEFT_LOW_LIMIT);
        const bool rightLow = HAL::MS::readBool(m_repo, Z_RIGHT_LOW_LIMIT);
        const bool backLow  = HAL::MS::readBool(m_repo, Z_BACK_LOW_LIMIT);

        _lowerToLimit(m_leftMotorBundle, leftLow, true, false);
        _lowerToLimit(m_rightMotorBundle, rightLow, true, false);
        _lowerToLimit(m_backMotorBundle, backLow, true, false);
    }

    // ======================
    // Helper methods
    // ======================

    void ZMotorsInitTask::_lowerToLimit(const z_motor_bundle_t &bundle,
                                        bool lowLimitReached,
                                        bool fineProfileNeeded,
                                        bool profileChanged)
    {
        const bool isMoving = bundle.motor->isMoving();

        if (lowLimitReached)
        {
            if (isMoving)
                bundle.motor->emergencyStop();
        }
        else
        {
            // Send command if not moving OR if moving but the profile changed
            if (!isMoving || profileChanged)
                bundle.motor->moveDirection(
                    HAL::Act::MotorDirection::Negative,
                    fineProfileNeeded ? bundle.fineProfile : bundle.fastProfile);
        }
    }

    void ZMotorsInitTask::_climbOutOfLimit(const z_motor_bundle_t &bundle, bool limitValue)
    {
        const bool isMoving = bundle.motor->isMoving();

        if (limitValue == isMoving) // No action needed
            return;

        if (!limitValue)
            bundle.motor->emergencyStop();
        else
            bundle.motor->moveDirection(HAL::Act::MotorDirection::Positive, bundle.fineProfile);
    }

}
