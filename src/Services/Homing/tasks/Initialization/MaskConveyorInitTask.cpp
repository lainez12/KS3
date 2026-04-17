#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Initialization/MaskConveyorInitTask.h>

namespace Kub3::Services
{

    MaskConveyorInitTask::MaskConveyorInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                               Shared<HAL::Act::IMotor> motor,
                                               Config::kinematic_profile_t fastProfile,
                                               Config::kinematic_profile_t fineProfile,
                                               Config::kinematic_profile_t contactProfile,
                                               double cm3InitOffsetMm) :
        MaskHomingTask(repo, motor, fastProfile, fineProfile, contactProfile),
        m_cm3InitOffsetMm(cm3InitOffsetMm)
    {}

    bool MaskConveyorInitTask::tick(void)
    {
        const bool done = MaskHomingTask::tick();

        if (done)
        {
            if (HAL::MS::readBool(m_repo, CM2))
                m_motor->resetEncoder(0.0); // CM2 init
            else
                m_motor->resetEncoder(m_cm3InitOffsetMm); // CM3 init
        }

        return done;
    }

}
