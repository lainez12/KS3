#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <Services/Stowage/tasks/StowageMoveMaskConvToUnconstrainedTask.h>

namespace Kub3::Services
{

    StowageMoveMaskConvToUnconstrainedTask::StowageMoveMaskConvToUnconstrainedTask(
        Shared<HAL::MS::IMachineStatusRepo> repo,
        const stowage_mask_motor_bundle_t &bundle) :
        m_repo(std::move(repo)),
        m_motorBundle(bundle)
    {
    }

    void StowageMoveMaskConvToUnconstrainedTask::start()
    {
        if (auto targetPosMmOpt = HAL::MS::tryRead<double>(m_repo, V_MASK_CONV_UNCONSTRAINED_POS_MM))
        {
            postInfo("Moving mask conveyor to unconstrained position.");
            m_targetPosMm = targetPosMmOpt.value();
            m_motorBundle.motor->moveAbsolute(m_targetPosMm, m_motorBundle.kinematics);
        }
        else
        {
            constexpr const char *err = "Failed to retrieve mask conveyor's unconstrained position.";

            postError(err);
            abort(err);
        }
    }

    bool StowageMoveMaskConvToUnconstrainedTask::tick()
    {
        const double currentPosMm = m_motorBundle.motor->getEncoderPositionMm();
        const double deltaMm      = std::fabs(currentPosMm - m_targetPosMm);
        const double toleranceMm  = m_motorBundle.motor->getPrecisionMm(m_motorBundle.kinematics) * 2;

        if (deltaMm < toleranceMm)
        {
            postInfo("Unconstrained position reached.");
            m_motorBundle.motor->emergencyStop();
            return true;
        }

        if (!m_motorBundle.motor->isMoving())
        {
            m_motorBundle.motor->moveAbsolute(m_targetPosMm, m_motorBundle.kinematics);
        }
        return false;
    }

}
