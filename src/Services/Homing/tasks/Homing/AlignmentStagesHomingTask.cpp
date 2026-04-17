#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Homing/AlignmentStagesHomingTask.h>

#define POSITION_TOLERANCE_MM 0.25

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
        handleSingleMotorLogic(m_xMotorBundle);
        handleSingleMotorLogic(m_yMotorBundle);
        handleSingleMotorLogic(m_thetaMotorBundle);
    }

    bool AlignmentStagesHomingTask::tick(void)
    {
        const double xStageCentered     = handleSingleMotorLogic(m_xMotorBundle);
        const double yStageCentered     = handleSingleMotorLogic(m_yMotorBundle);
        const double thetaStageCentered = handleSingleMotorLogic(m_thetaMotorBundle);

        return xStageCentered && yStageCentered && thetaStageCentered;
    }

    bool AlignmentStagesHomingTask::handleSingleMotorLogic(const stage_motor_bundle_t &bundle)
    {
        const double currentPosMm = bundle.motor->getEncoderPositionMm();
        const bool centered       = fabs(currentPosMm - bundle.centerPositionMm) < POSITION_TOLERANCE_MM;
        const bool actionNeeded   = (centered == bundle.motor->isMoving());

        if (!actionNeeded)
            return centered;

        if (centered)
            bundle.motor->emergencyStop();
        else
            bundle.motor->moveAbsolute(bundle.centerPositionMm, bundle.kinematic);

        return centered;
    }

}
