#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Initialization/AlignmentStagesInitTask.h>

namespace Kub3::Services
{

    AlignmentStagesInitTask::AlignmentStagesInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                                     Shared<HAL::Act::IPositionMotor> xStageMotor,
                                                     Shared<HAL::Act::IPositionMotor> yStageMotor,
                                                     Shared<HAL::Act::IPositionMotor> thetaStageMotor,
                                                     Config::kinematic_profile_t xStageProfile,
                                                     Config::kinematic_profile_t yStageProfile,
                                                     Config::kinematic_profile_t thetaStageProfile) :
        m_repo(std::move(repo)),
        m_xStageMotor(std::move(xStageMotor)),
        m_yStageMotor(std::move(yStageMotor)),
        m_thetaStageMotor(std::move(thetaStageMotor)),
        m_xStageProfile(std::move(xStageProfile)),
        m_yStageProfile(std::move(yStageProfile)),
        m_thetaStageProfile(std::move(thetaStageProfile))
    {}

    void AlignmentStagesInitTask::start(void)
    {
        const bool xStageInitLimit     = HAL::MS::readBool(m_repo, X_STAGE_LEFT_LIMIT);
        const bool yStageInitLimit     = HAL::MS::readBool(m_repo, Y_STAGE_BACK_LIMIT);
        const bool thetaStageInitLimit = HAL::MS::readBool(m_repo, THETA_STAGE_ANTI_CLOCKWISE_LIMIT);

        // Ensure motors are stopped
        stopMotorIfMoving(m_xStageMotor);
        stopMotorIfMoving(m_yStageMotor);
        stopMotorIfMoving(m_thetaStageMotor);

        // Start moving if needed
        handleSingleMotorLogic(m_xStageMotor, m_xStageProfile, xStageInitLimit);
        handleSingleMotorLogic(m_yStageMotor, m_yStageProfile, yStageInitLimit);
        handleSingleMotorLogic(m_thetaStageMotor, m_thetaStageProfile, thetaStageInitLimit);
    }

    bool AlignmentStagesInitTask::tick(void)
    {
        const bool xStageInitLimit     = HAL::MS::readBool(m_repo, X_STAGE_LEFT_LIMIT);
        const bool yStageInitLimit     = HAL::MS::readBool(m_repo, Y_STAGE_BACK_LIMIT);
        const bool thetaStageInitLimit = HAL::MS::readBool(m_repo, THETA_STAGE_ANTI_CLOCKWISE_LIMIT);

        handleSingleMotorLogic(m_xStageMotor, m_xStageProfile, xStageInitLimit);
        handleSingleMotorLogic(m_yStageMotor, m_yStageProfile, yStageInitLimit);
        handleSingleMotorLogic(m_thetaStageMotor, m_thetaStageProfile, thetaStageInitLimit);

        // Initialization limits reached and motors stopped, reset encoders
        if (xStageInitLimit && yStageInitLimit && thetaStageInitLimit)
        {
            m_xStageMotor->resetEncoder(0.0);
            m_yStageMotor->resetEncoder(0.0);
            m_thetaStageMotor->resetEncoder(0.0);
            return true;
        }
        return false;
    }

    void AlignmentStagesInitTask::stopMotorIfMoving(Shared<HAL::Act::IPositionMotor> motor)
    {
        if (motor && motor->isMoving())
            motor->emergencyStop();
    }

    // Stops motor if `motor->isMoving()` returns `true` and `limitValue` is `true`.
    // Starts motor if not `limitValue` is `false` and `motor->isMoving()` returns `false`.
    void AlignmentStagesInitTask::handleSingleMotorLogic(Shared<HAL::Act::IPositionMotor> motor, Config::kinematic_profile_t kineProfile, bool limitValue)
    {
        if (!motor)
            return;

        // - limit reached AND motor moving => needs to stop
        // - limit not reached AND motor not moving => needs to move
        const bool actionNeeded = (motor->isMoving() == limitValue); // true if states differ

        if (!actionNeeded)
            return; // nothing to do

        if (limitValue)
            motor->emergencyStop();
        else
            motor->moveDirection(HAL::Act::MotorDirection::Positive, kineProfile);
    }
}
