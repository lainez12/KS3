#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Initialization/WaferConveyorInitTask.h>

namespace Kub3::Services
{

    WaferConveyorInitTask::WaferConveyorInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                                 Shared<HAL::Act::IPositionMotor> motor,
                                                 Config::kinematic_profile_t fastProfile,
                                                 Config::kinematic_profile_t fineProfile) :
        WaferHomingTask(repo, motor, fastProfile, fineProfile) {}

    bool WaferConveyorInitTask::tick(void)
    {
        const bool done = WaferHomingTask::tick();

        if (done)
            m_motor->resetEncoder(0.0);
        return done;
    }

}
