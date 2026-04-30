#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/tasks/ToggleValveTask.h>

namespace Kub3::Services
{

    ToggleValveTask::ToggleValveTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                                     Shared<HAL::Act::IValve> valve,
                                     bool targetValveOpenState,
                                     std::string targetSensor,
                                     bool targetSensorState) :
        m_repo(std::move(repo)),
        m_valve(std::move(valve)),
        m_targetValveOpenState(targetValveOpenState),
        m_targetSensor(std::move(targetSensor)),
        m_targetSensorState(targetSensorState) {}

    void ToggleValveTask::start()
    {
    }

    bool ToggleValveTask::tick()
    {
        const bool targetStateReached = (HAL::MS::readBool(m_repo, m_targetSensor) == m_targetSensorState);

        if (!targetStateReached)
            toggleValveIfNeeded();
        return targetStateReached;
    }

    void ToggleValveTask::toggleValveIfNeeded(void)
    {
        if (m_valve->isOpen() != m_targetValveOpenState)
        {
            if (m_targetValveOpenState) // Request is to open valve
                m_valve->open();
            else // Request is to close valve
                m_valve->close();
        }
    }

}
