#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Homing/CamerasHomingTask.h>

#define POSITION_TOLERANCE_MM 0.25

namespace Kub3::Services
{

    CamerasHomingTask::CamerasHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                         homing_cam_bundle_t leftCamXBundle,
                                         homing_cam_bundle_t leftCamYBundle,
                                         homing_cam_bundle_t rightCamXBundle,
                                         homing_cam_bundle_t rightCamYBundle) :
        m_repo(std::move(repo)),
        m_leftCamXBundle(std::move(leftCamXBundle)),
        m_leftCamYBundle(std::move(leftCamYBundle)),
        m_rightCamXBundle(std::move(rightCamXBundle)),
        m_rightCamYBundle(std::move(rightCamYBundle)) {}

    void CamerasHomingTask::start(void)
    {
    }

    bool CamerasHomingTask::tick(void)
    {
        const double leftCamXCentered  = handleSingleMotorLogic(m_leftCamXBundle);
        const double leftCamYCentered  = handleSingleMotorLogic(m_leftCamYBundle);
        const double rightCamXCentered = handleSingleMotorLogic(m_rightCamXBundle);
        const double rightCamYCentered = handleSingleMotorLogic(m_rightCamYBundle);

        return leftCamXCentered && leftCamYCentered && rightCamXCentered && rightCamYCentered;
    }

    bool CamerasHomingTask::handleSingleMotorLogic(const homing_cam_bundle_t &bundle)
    {
        const double currentPosMm = bundle.motor->getEncoderPositionMm();
        const bool centered       = fabs(currentPosMm - bundle.centerPositionMm) < bundle.motor->getPrecisionMm(bundle.kinematicProfile);
        const bool actionNeeded   = (centered == bundle.motor->isMoving());

        qDebug().noquote() << QString("[CamerasHomingTask] %1 centered: %2 (current pos: %3, target pos: %4)")
                                  .arg(bundle.motor->getId().data())
                                  .arg(centered)
                                  .arg(currentPosMm)
                                  .arg(bundle.centerPositionMm);

        if (!actionNeeded)
            return centered;

        if (centered)
            bundle.motor->emergencyStop();
        else
            bundle.motor->moveAbsolute(bundle.centerPositionMm, bundle.kinematicProfile);

        return centered;
    }

}
