#include <ViewModels/ViewModelsSettings/TemperatureViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    TemperatureViewModel::TemperatureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    TemperatureViewModel::~TemperatureViewModel() {
    }

    void TemperatureViewModel::loadConnections(void) {
    }

    void TemperatureViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewsModelSettings