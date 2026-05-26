#include <ViewModels/ViewModelsSettings/VersionViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    VersionViewModel::VersionViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    VersionViewModel::~VersionViewModel() {
    }

    void VersionViewModel::loadConnections(void) {
    }

    void VersionViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewsModelSettings