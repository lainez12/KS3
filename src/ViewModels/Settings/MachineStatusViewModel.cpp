#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <ViewModels/Settings/MachineStatusViewModel.h>

namespace Kub3::UI::ViewModels
{

    MachineStatusViewModel::MachineStatusViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseVisionViewModel(parent),
        m_repo(std::move(repo))
    {
    }

    void MachineStatusViewModel::loadConnections(void)
    {
        BaseViewModel::loadConnections();

        for (const std::string &key : m_repo->getRegisteredKeys())
        {
            ps_handleSensorValueChanged(key);
        }
    }

    void MachineStatusViewModel::ps_handleSensorValueChanged(const std::string &key)
    {
        Optional<HAL::MS::MachineValue> valueOpt = m_repo->getValueRaw(key);

        if (!valueOpt.has_value())
            return;

        auto qKey   = QString::fromStdString(key);
        auto museum = overloadedCallable(
            [&](bool v) { emit s_booleanSensorUpdate(qKey, v); },
            [&](int32_t v) { emit s_integerSensorUpdate(qKey, v); },
            [&](uint16_t v) { emit s_unsignedIntegerSensorUpdate(qKey, v); },
            [&](uint32_t v) { emit s_unsignedIntegerSensorUpdate(qKey, v); },
            [&](auto) { qWarning() << "[MachineStatusViewModel] unknown sensor changed notification received."; });

        std::visit(museum, valueOpt.value());
    }

}
