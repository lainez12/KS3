#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Initialization/CamerasInitTask.h>

namespace Kub3::Services
{

    CamerasInitTask::CamerasInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                     const Config::process_config_t &processConfig,
                                     Shared<HAL::Act::IMotor> leftCamXMotor,
                                     Shared<HAL::Act::IMotor> leftCamYMotor,
                                     Shared<HAL::Act::IMotor> rightCamXMotor,
                                     Shared<HAL::Act::IMotor> rightCamYMotor,
                                     Config::kinematic_profile_t kinematicProfile) :
        m_repo(std::move(repo)),
        m_processConfig(processConfig),
        m_leftCamXMotor(std::move(leftCamXMotor)),
        m_leftCamYMotor(std::move(leftCamYMotor)),
        m_rightCamXMotor(std::move(rightCamXMotor)),
        m_rightCamYMotor(std::move(rightCamYMotor)),
        m_kinematicProfile(std::move(kinematicProfile)) {}

    void CamerasInitTask::start(void)
    {
        const bool leftCamXLimitVal  = HAL::MS::readBool(m_repo, LEFT_CAMERA_X_LEFT_LIMIT);
        const bool leftCamYLimitVal  = HAL::MS::readBool(m_repo, LEFT_CAMERA_X_RIGHT_LIMIT);
        const bool rightCamXLimitVal = HAL::MS::readBool(m_repo, LEFT_CAMERA_Y_FRONT_LIMIT);
        const bool rightCamYLimitVal = HAL::MS::readBool(m_repo, LEFT_CAMERA_Y_BACK_LIMIT);

        // Ensure motors are stopped
        stopMotorIfMoving(m_leftCamXMotor);
        stopMotorIfMoving(m_leftCamYMotor);
        stopMotorIfMoving(m_rightCamXMotor);
        stopMotorIfMoving(m_rightCamYMotor);

        // Start moving if needed
        handleSingleMotorLogic(m_leftCamXMotor, leftCamXLimitVal);
        handleSingleMotorLogic(m_leftCamYMotor, leftCamYLimitVal);
        handleSingleMotorLogic(m_rightCamXMotor, rightCamXLimitVal);
        handleSingleMotorLogic(m_rightCamYMotor, rightCamYLimitVal);
    }

    bool CamerasInitTask::tick(void)
    {
        const bool leftCamXLimitVal  = HAL::MS::readBool(m_repo, LEFT_CAMERA_X_LEFT_LIMIT);
        const bool leftCamYLimitVal  = HAL::MS::readBool(m_repo, LEFT_CAMERA_X_RIGHT_LIMIT);
        const bool rightCamXLimitVal = HAL::MS::readBool(m_repo, LEFT_CAMERA_Y_FRONT_LIMIT);
        const bool rightCamYLimitVal = HAL::MS::readBool(m_repo, LEFT_CAMERA_Y_BACK_LIMIT);

        handleSingleMotorLogic(m_leftCamXMotor, leftCamXLimitVal);
        handleSingleMotorLogic(m_leftCamYMotor, leftCamYLimitVal);
        handleSingleMotorLogic(m_rightCamXMotor, rightCamXLimitVal);
        handleSingleMotorLogic(m_rightCamYMotor, rightCamYLimitVal);

        // Initialization limits reached and motors stopped, reset encoders using config values (in mm)
        if (leftCamXLimitVal && leftCamYLimitVal && rightCamXLimitVal && rightCamYLimitVal)
        {
            m_leftCamXMotor->resetEncoder(m_processConfig.left_cam_x_reset_pos_mm);
            m_leftCamYMotor->resetEncoder(m_processConfig.left_cam_y_reset_pos_mm);
            m_rightCamXMotor->resetEncoder(m_processConfig.right_cam_x_reset_pos_mm);
            m_rightCamYMotor->resetEncoder(m_processConfig.right_cam_y_reset_pos_mm);
            return true;
        }
        return false;
    }

    void CamerasInitTask::stopMotorIfMoving(Shared<HAL::Act::IMotor> motor)
    {
        if (motor && motor->isMoving())
            motor->emergencyStop();
    }

    // Stops motor if `motor->isMoving()` returns `true` and `limitValue` is `true`.
    // Starts motor if not `limitValue` is `false` and `motor->isMoving()` returns `false`.
    void CamerasInitTask::handleSingleMotorLogic(Shared<HAL::Act::IMotor> motor, bool limitValue)
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
            motor->moveDirection(HAL::Act::MotorDirection::Positive, m_kinematicProfile);
    }

}
