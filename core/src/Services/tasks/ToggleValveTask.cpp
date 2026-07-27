#include <cctype>

#include <HAL/MachineStatus/actuators_labels.h>
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
        const auto valveId = m_valve->getId();
        std::string valveName;

        if (valveId == MASK_VACUUM_VALVE)
            valveName = "mask vacuum";
        else if (valveId == WAFER_VACUUM_VALVE)
            valveName = "wafer vacuum";
        else if (valveId == WAFER_COMPRESSED_AIR_VALVE)
            valveName = "compressed air";
        else
        {
            qCritical() << "Toggling anonymous valve. Abnormal behaviour.";
            return;
        }

        auto capitalizeFirst = [](std::string str) {
            if (!str.empty())
                str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[0])));
            return str;
        };

        if (HAL::MS::readBool(m_repo, m_targetSensor) == m_targetSensorState)
            postInfo(std::format("{} already {}. Skipping.", capitalizeFirst(valveName), m_targetSensorState ? "active" : "disabled"));
        else
            postInfo(std::format("{} {}...", (m_targetSensorState ? "Enabling" : "Disabling"), valveName));
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
