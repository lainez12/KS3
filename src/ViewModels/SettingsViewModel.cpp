#include <ViewModels/SettingsViewModel.h>

namespace Kub3::UI::ViewModels {
    SettingsViewModel::SettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    SettingsViewModel::~SettingsViewModel() {
    }

    void SettingsViewModel::loadConnections(void) {
    }

    void SettingsViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels
