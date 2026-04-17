#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Initialization/DeckInitTask.h>

namespace Kub3::Services
{

    DeckInitTask::DeckInitTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                               Shared<HAL::Act::IMotor> deckMotor,
                               Config::kinematic_profile_t kinematicProfile) :
        DeckHomingTask(repo, deckMotor, kinematicProfile)
    {}

    bool DeckInitTask::tick(void)
    {
        const bool done = DeckHomingTask::tick();

        if (done)
            m_deckMotor->resetEncoder(0.0);
        return done;
    }

}
