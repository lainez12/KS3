#pragma once

#include "../Homing/WaferHomingTask.h"

namespace Kub3::Services
{

    class WaferConveyorInitTask final : public WaferHomingTask
    {
    public:
        WaferConveyorInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                              Shared<HAL::Act::IMotor> motor,
                              Config::kinematic_profile_t fastProfile,
                              Config::kinematic_profile_t fineProfile);

        bool tick(void) override;
    };

}
