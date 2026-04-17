#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <utils.h>

#include "../Homing/DeckHomingTask.h"

namespace Kub3::Services
{

    class DeckInitTask final : public DeckHomingTask
    {
    public:
        DeckInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                     Shared<HAL::Act::IMotor> deckMotor,
                     Config::kinematic_profile_t kinematicProfile);

        bool tick(void) override;
    };

}
