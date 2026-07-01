#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <Services/Homing/tasks/Homing/AlignmentStagesHomingTask.h>

namespace Kub3::Services
{

    AlignmentStagesHomingTask::AlignmentStagesHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                                         stage_motor_bundle_t xMotorBundle,
                                                         stage_motor_bundle_t yMotorBundle,
                                                         stage_motor_bundle_t thetaMotorBundle) :
        m_repo(std::move(repo)),
        m_xMotorBundle(std::move(xMotorBundle)),
        m_yMotorBundle(std::move(yMotorBundle)),
        m_thetaMotorBundle(std::move(thetaMotorBundle)) {}

    void AlignmentStagesHomingTask::start(void)
    {
        const bool z1 = HAL::MS::readBool(m_repo, Z1);
        const bool z2 = HAL::MS::readBool(m_repo, Z2);

        // Between Z1 and Z2 is the danger zone.
        // TODO: Maybe reject demand a bit less aggressively
        if (z1 && !z2)
            throw std::runtime_error("CRITICAL: Attempted to move alignment stages while Z is inside the collision zone.");

        handleSingleMotorLogic(m_xMotorBundle, readCenterPosition(m_xMotorBundle, V_X_STAGE_CENTER_MM));
        handleSingleMotorLogic(m_yMotorBundle, readCenterPosition(m_yMotorBundle, V_Y_STAGE_CENTER_MM));
        handleSingleMotorLogic(m_thetaMotorBundle, readCenterPosition(m_thetaMotorBundle, V_THETA_STAGE_CENTER_MM));
    }

    bool AlignmentStagesHomingTask::tick(void)
    {
        const bool xStageCentered     = handleSingleMotorLogic(m_xMotorBundle, readCenterPosition(m_xMotorBundle, V_X_STAGE_CENTER_MM));
        const bool yStageCentered     = handleSingleMotorLogic(m_yMotorBundle, readCenterPosition(m_yMotorBundle, V_Y_STAGE_CENTER_MM));
        const bool thetaStageCentered = handleSingleMotorLogic(m_thetaMotorBundle, readCenterPosition(m_thetaMotorBundle, V_THETA_STAGE_CENTER_MM));

        return xStageCentered && yStageCentered && thetaStageCentered;
    }

    bool AlignmentStagesHomingTask::handleSingleMotorLogic(const stage_motor_bundle_t &bundle, const double centerPosMm)
    {
        const double currentPosMm = bundle.motor->getEncoderPositionMm();
        const bool centered       = fabs(currentPosMm - centerPosMm) < bundle.motor->getPrecisionMm(bundle.kinematic);
        const bool actionNeeded   = (centered == bundle.motor->isMoving());

        if (!actionNeeded)
            return centered;

        if (centered)
            bundle.motor->emergencyStop();
        else
            bundle.motor->moveAbsolute(centerPosMm, bundle.kinematic);

        return centered;
    }

    double AlignmentStagesHomingTask::readCenterPosition(const stage_motor_bundle_t &bundle, const char *label)
    {
        if (auto centerOpt = HAL::MS::tryReadDouble(m_repo, label))
        {
            return *centerOpt;
        }

        qWarning().noquote() << QString("Failed to read dynamically computed alignment stage center (%1)."
                                        " Falling back to configuration value.")
                                    .arg(label);
        return bundle.centerPositionMm;
    }

}
