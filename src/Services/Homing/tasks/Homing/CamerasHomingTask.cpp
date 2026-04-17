#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Homing/CamerasHomingTask.h>

#define POSITION_TOLERANCE_MM 0.25

namespace Kub3::Services
{

    CamerasHomingTask::CamerasHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                         camera_motor_bundle_t leftCamXBundle,
                                         camera_motor_bundle_t leftCamYBundle,
                                         camera_motor_bundle_t rightCamXBundle,
                                         camera_motor_bundle_t rightCamYBundle,
                                         Config::kinematic_profile_t kinematicProfile) :
        m_repo(std::move(repo)),
        m_leftCamXBundle(std::move(leftCamXBundle)),
        m_leftCamYBundle(std::move(leftCamYBundle)),
        m_rightCamXBundle(std::move(rightCamXBundle)),
        m_rightCamYBundle(std::move(rightCamYBundle)),
        m_kinematicProfile(kinematicProfile) {}

    void CamerasHomingTask::start(void)
    {
        handleSingleMotorLogic(m_leftCamXBundle);
        handleSingleMotorLogic(m_leftCamYBundle);
        handleSingleMotorLogic(m_rightCamXBundle);
        handleSingleMotorLogic(m_rightCamYBundle);
    }

    bool CamerasHomingTask::tick(void)
    {
        const double leftCamXCentered  = handleSingleMotorLogic(m_leftCamXBundle);
        const double leftCamYCentered  = handleSingleMotorLogic(m_leftCamYBundle);
        const double rightCamXCentered = handleSingleMotorLogic(m_rightCamXBundle);
        const double rightCamYCentered = handleSingleMotorLogic(m_rightCamYBundle);

        return leftCamXCentered && leftCamYCentered && rightCamXCentered && rightCamYCentered;
    }

    bool CamerasHomingTask::handleSingleMotorLogic(const camera_motor_bundle_t &bundle)
    {
        const double currentPosMm = bundle.motor->getEncoderPositionMm();
        const bool centered       = fabs(currentPosMm - bundle.centerPositionMm) < POSITION_TOLERANCE_MM;
        const bool actionNeeded   = (centered == bundle.motor->isMoving());

        if (!actionNeeded)
            return centered;

        if (centered)
            bundle.motor->emergencyStop();
        else
            bundle.motor->moveAbsolute(bundle.centerPositionMm, m_kinematicProfile);

        return centered;
    }

}
