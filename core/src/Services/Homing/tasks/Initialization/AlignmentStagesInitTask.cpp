#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
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
        // Reset task initial state
        m_xPhase     = Phase::POSITIVE_SEARCH;
        m_yPhase     = Phase::POSITIVE_SEARCH;
        m_thetaPhase = Phase::POSITIVE_SEARCH;

        const bool xStageLimit     = HAL::MS::readBool(m_repo, X_STAGE_LEFT_LIMIT);
        const bool yStageLimit     = HAL::MS::readBool(m_repo, Y_STAGE_BACK_LIMIT);
        const bool thetaStageLimit = HAL::MS::readBool(m_repo, THETA_STAGE_ANTI_CLOCKWISE_LIMIT);

        // Ensure motors are stopped
        stopMotorIfMoving(m_xStageMotor);
        stopMotorIfMoving(m_yStageMotor);
        stopMotorIfMoving(m_thetaStageMotor);

        // Start moving if needed
        handleSingleMotorLogic(m_xStageMotor, m_xStageProfile, m_xPhase, xStageLimit);
        handleSingleMotorLogic(m_yStageMotor, m_yStageProfile, m_yPhase, yStageLimit);
        handleSingleMotorLogic(m_thetaStageMotor, m_thetaStageProfile, m_thetaPhase, thetaStageLimit);
    }

    bool AlignmentStagesInitTask::tick(void)
    {
        const char *xLimitId     = (m_xPhase == Phase::POSITIVE_SEARCH) ? X_STAGE_LEFT_LIMIT : X_STAGE_RIGHT_LIMIT;
        const char *yLimitId     = (m_yPhase == Phase::POSITIVE_SEARCH) ? Y_STAGE_BACK_LIMIT : Y_STAGE_FRONT_LIMIT;
        const char *thetaLimitId = (m_thetaPhase == Phase::POSITIVE_SEARCH) ? THETA_STAGE_ANTI_CLOCKWISE_LIMIT : THETA_STAGE_CLOCKWISE_LIMIT;

        const bool xStageLimit     = HAL::MS::readBool(m_repo, xLimitId);
        const bool yStageLimit     = HAL::MS::readBool(m_repo, yLimitId);
        const bool thetaStageLimit = HAL::MS::readBool(m_repo, thetaLimitId);

        handleSingleMotorLogic(m_xStageMotor, m_xStageProfile, m_xPhase, xStageLimit);
        handleSingleMotorLogic(m_yStageMotor, m_yStageProfile, m_yPhase, yStageLimit);
        handleSingleMotorLogic(m_thetaStageMotor, m_thetaStageProfile, m_thetaPhase, thetaStageLimit);

        // Initialization limits reached and motors stopped, reset encoders
        if (m_xPhase == Phase::DONE && m_yPhase == Phase::DONE && m_thetaPhase == Phase::DONE)
        {
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
    void AlignmentStagesInitTask::handleSingleMotorLogic(
        Shared<HAL::Act::IPositionMotor> motor,
        Config::kinematic_profile_t kineProfile,
        Phase &stagePhase,
        bool limitValue)
    {
        if (!motor || stagePhase == Phase::DONE)
            return;

        if (limitValue)
        {
            if (motor->isMoving())
                motor->emergencyStop();

            if (stagePhase == Phase::POSITIVE_SEARCH)
            {
                motor->resetEncoder(0.0);
                stagePhase = Phase::NEGATIVE_SEARCH;
            }
            else if (stagePhase == Phase::NEGATIVE_SEARCH)
            {
                const double centerPos = motor->getEncoderPositionMm() / 2.0;

                if (motor == m_xStageMotor)
                    m_repo->setValueRaw(V_X_STAGE_CENTER_MM, centerPos);
                else if (motor == m_yStageMotor)
                    m_repo->setValueRaw(V_Y_STAGE_CENTER_MM, centerPos);
                else if (motor == m_thetaStageMotor)
                    m_repo->setValueRaw(V_THETA_STAGE_CENTER_MM, centerPos);

                stagePhase = Phase::DONE;
            }
        }
        else
        {
            if (!motor->isMoving())
            {
                auto dir = (stagePhase == Phase::POSITIVE_SEARCH)
                               ? HAL::Act::MotorDirection::Positive
                               : HAL::Act::MotorDirection::Negative;
                motor->moveDirection(dir, kineProfile);
            }
        }
    }

}
