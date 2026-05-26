#include <ViewModels/ViewModelsSettings/UpdateSoftwareViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    UpdateSoftwareViewModel::UpdateSoftwareViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    UpdateSoftwareViewModel::~UpdateSoftwareViewModel() {
    }

    void UpdateSoftwareViewModel::loadConnections(void) {
    }

    void UpdateSoftwareViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewsModelSettings