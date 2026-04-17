#pragma once

#include "../Homing/MaskHomingTask.h"

namespace Kub3::Services
{

    class MaskConveyorInitTask final : public MaskHomingTask
    {
    public:
        MaskConveyorInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                             Shared<HAL::Act::IMotor> motor,
                             Config::kinematic_profile_t fastProfile,
                             Config::kinematic_profile_t fineProfile,
                             Config::kinematic_profile_t contactProfile,
                             double cm3InitOffsetMm);

        bool tick(void) override;

    private:
        double m_cm3InitOffsetMm = 0.0;
    };

}
