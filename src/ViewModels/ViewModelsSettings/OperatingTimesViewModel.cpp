#include <ViewModels/ViewModelsSettings/OperatingTimesViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    OperatingTimesViewModel::OperatingTimesViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    OperatingTimesViewModel::~OperatingTimesViewModel() {
    }

    void OperatingTimesViewModel::loadConnections(void) {
    }

    void OperatingTimesViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewsModelSettings