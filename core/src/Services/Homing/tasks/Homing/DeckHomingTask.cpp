#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Homing/tasks/Homing/DeckHomingTask.h>

namespace Kub3::Services
{

    DeckHomingTask::DeckHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                   Shared<HAL::Act::IMotor> deckMotor,
                                   Config::kinematic_profile_t kinematicProfile) :
        m_repo(repo),
        m_deckMotor(deckMotor),
        m_kinematicProfile(kinematicProfile) {}

    void DeckHomingTask::start(void)
    {
        const bool deckBackLimit = HAL::MS::readBool(m_repo, DECK_BACK_LIMIT);

        if (m_deckMotor && m_deckMotor->isMoving())
            m_deckMotor->emergencyStop();

        if (!deckBackLimit)
            moveTowardsLimit();
    }

    bool DeckHomingTask::tick(void)
    {
        const bool deckBackLimit = HAL::MS::readBool(m_repo, DECK_BACK_LIMIT);

        if (deckBackLimit)
        {
            m_deckMotor->emergencyStop();
            return true;
        }

        moveTowardsLimit();
        return false;
    }

    // TODO: `moveTowardsLimit` does not mean shit, limits are both ways here
    void DeckHomingTask::moveTowardsLimit(void)
    {
        if (m_deckMotor && !m_deckMotor->isMoving())
            m_deckMotor->moveDirection(HAL::Act::MotorDirection::Negative, m_kinematicProfile);
    }

}
