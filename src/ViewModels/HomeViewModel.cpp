#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <ViewModels/HomeViewModel.h>

namespace Kub3::UI::ViewModels {

    HomeViewModel::HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }

    HomeViewModel::~HomeViewModel() {
    }

    void HomeViewModel::loadConnections(void) {
        HAL::MS::IMachineStatusRepo *repo = m_repo.get();

        if (!repo)
            return;

        // Connect machine status repository to view model
        // connect(repo, &HAL::MS::IMachineStatusRepo::s_sensorValueChanged, this, &MachineStatusViewModel::handleSensorValueChanged);
    }

    void HomeViewModel::unloadConnections(void) {
        HAL::MS::IMachineStatusRepo *repo = m_repo.get();

        if (!repo)
            return;

        // Disonnect machine status repository to view model
        // disconnect(repo, &HAL::MS::IMachineStatusRepo::s_sensorValueChanged, this, &MachineStatusViewModel::handleSensorValueChanged);
    }
}
