#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class DeckHomingTask : public ITask
    {
    public:
        DeckHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                       Shared<HAL::Act::IMotor> deckMotor,
                       Config::kinematic_profile_t kinematicProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        void moveTowardsLimit(void);

    protected:
        Shared<HAL::Act::IMotor> m_deckMotor;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Config::kinematic_profile_t m_kinematicProfile;
    };

}
