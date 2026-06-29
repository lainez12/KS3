#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Initialization/CamerasInitTask.h>

namespace Kub3::Services
{

    CamerasInitTask::CamerasInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                     const Config::process_config_t &processConfig,
                                     init_cam_bundle_t leftCamXBundle,
                                     init_cam_bundle_t leftCamYBundle,
                                     init_cam_bundle_t rightCamXBundle,
                                     init_cam_bundle_t rightCamYBundle) :
        m_repo(std::move(repo)),
        m_processConf(processConfig),
        m_leftCamXBundle(std::move(leftCamXBundle)),
        m_leftCamYBundle(std::move(leftCamYBundle)),
        m_rightCamXBundle(std::move(rightCamXBundle)),
        m_rightCamYBundle(std::move(rightCamYBundle)) {}

    void CamerasInitTask::start(void)
    {
        qDebug() << "[CamerasInitTask] start";
        const bool leftCamXLimitVal  = HAL::MS::readBool(m_repo, LEFT_CAMERA_X_LEFT_LIMIT);
        const bool leftCamYLimitVal  = HAL::MS::readBool(m_repo, LEFT_CAMERA_X_RIGHT_LIMIT);
        const bool rightCamXLimitVal = HAL::MS::readBool(m_repo, LEFT_CAMERA_Y_FRONT_LIMIT);
        const bool rightCamYLimitVal = HAL::MS::readBool(m_repo, LEFT_CAMERA_Y_BACK_LIMIT);

        // Ensure motors are stopped
        stopMotorIfMoving(m_leftCamXBundle.motor);
        stopMotorIfMoving(m_leftCamYBundle.motor);
        stopMotorIfMoving(m_rightCamXBundle.motor);
        stopMotorIfMoving(m_rightCamYBundle.motor);
    }

    bool CamerasInitTask::tick(void)
    {
        const bool leftCamXLimitVal  = HAL::MS::readBool(m_repo, LEFT_CAMERA_X_LEFT_LIMIT);
        const bool leftCamYLimitVal  = HAL::MS::readBool(m_repo, LEFT_CAMERA_Y_FRONT_LIMIT);
        const bool rightCamXLimitVal = HAL::MS::readBool(m_repo, RIGHT_CAMERA_X_RIGHT_LIMIT);
        const bool rightCamYLimitVal = HAL::MS::readBool(m_repo, RIGHT_CAMERA_Y_FRONT_LIMIT);
        qDebug() << QString("[CamerasInitTask] tick LX: %1, LY: %2, RX: %3, RY: %4")
                        .arg(leftCamXLimitVal)
                        .arg(leftCamYLimitVal)
                        .arg(rightCamXLimitVal)
                        .arg(rightCamYLimitVal);

        handleSingleMotorLogic(m_leftCamXBundle, leftCamXLimitVal);
        handleSingleMotorLogic(m_leftCamYBundle, leftCamYLimitVal);
        handleSingleMotorLogic(m_rightCamXBundle, rightCamXLimitVal);
        handleSingleMotorLogic(m_rightCamYBundle, rightCamYLimitVal);

        // Initialization limits reached and motors stopped, reset encoders using config values (in mm)
        if (leftCamXLimitVal && leftCamYLimitVal && rightCamXLimitVal && rightCamYLimitVal)
        {
            m_leftCamXBundle.motor->resetEncoder(m_processConf.vision.left_cam_x_reset_pos_mm);
            m_leftCamYBundle.motor->resetEncoder(m_processConf.vision.left_cam_y_reset_pos_mm);
            m_rightCamXBundle.motor->resetEncoder(m_processConf.vision.right_cam_x_reset_pos_mm);
            m_rightCamYBundle.motor->resetEncoder(m_processConf.vision.right_cam_y_reset_pos_mm);
            return true;
        }
        return false;
    }

    void CamerasInitTask::stopMotorIfMoving(Shared<HAL::Act::IPositionMotor> motor)
    {
        if (motor && motor->isMoving())
            motor->emergencyStop();
    }

    // Stops motor if `motor->isMoving()` returns `true` and `limitValue` is `true`.
    // Starts motor if not `limitValue` is `false` and `motor->isMoving()` returns `false`.
    void CamerasInitTask::handleSingleMotorLogic(const init_cam_bundle_t &bundle, bool limitValue)
    {
        if (!bundle.motor)
            return;

        // - limit reached AND motor moving => needs to stop
        // - limit not reached AND motor not moving => needs to move
        const bool actionNeeded = (bundle.motor->isMoving() == limitValue); // true if states differ

        if (!actionNeeded)
            return; // nothing to do

        if (limitValue)
            bundle.motor->emergencyStop();
        else
        {
            auto motorId = bundle.motor->getId();
            // Motors initialize their encoder on their external limit:
            // - Left X has to move negative and Right X positive
            // - Both Ys have to move negative
            auto direction =
                (motorId == RIGHT_CAMERA_X_MOTOR) ? HAL::Act::MotorDirection::Positive : HAL::Act::MotorDirection::Negative;

            bundle.motor->moveDirection(direction, bundle.kinematicProfile);
        }
    }

}
