#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <ViewModels/MachineStatusViewModel.h>

namespace Kub3::UI::ViewModels
{

    MachineStatusViewModel::MachineStatusViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    MachineStatusViewModel::~MachineStatusViewModel()
    {
    }

    void MachineStatusViewModel::loadConnections(void)
    {
        HAL::MS::IMachineStatusRepo *repo = m_repo.get();

        if (!repo)
            return;

        // Connect machine status repository to view model
        connect(repo, &HAL::MS::IMachineStatusRepo::s_sensorValueChanged, this, &MachineStatusViewModel::handleSensorValueChanged);
    }

    void MachineStatusViewModel::unloadConnections(void)
    {
        HAL::MS::IMachineStatusRepo *repo = m_repo.get();

        if (!repo)
            return;

        // Disonnect machine status repository to view model
        disconnect(repo, &HAL::MS::IMachineStatusRepo::s_sensorValueChanged, this, &MachineStatusViewModel::handleSensorValueChanged);
    }

    void MachineStatusViewModel::handleSensorValueChanged(const QString &key)
    {
        using SensorValue = HAL::MS::SensorValue;

        Optional<SensorValue> valueOpt = m_repo->getSensorRaw(key.toStdString());

        if (!valueOpt.has_value())
            return;

        SensorValue value = valueOpt.value();

        auto visitor = overloadedCallable(
            [&](bool v) { emit s_booleanSensorUpdate(key, v); },
            [&](int32_t v) { emit s_integerSensorUpdate(key, v); },
            [&](uint16_t v) { emit s_unsignedIntegerSensorUpdate(key, v); },
            [&](uint32_t v) { emit s_unsignedIntegerSensorUpdate(key, v); },
            [&](auto) { qWarning() << "[MachineStatusViewModel] unknown sensor changed notification received."; });

        std::visit(visitor, value);
    }
}
