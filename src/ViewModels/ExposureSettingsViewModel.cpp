#include <ViewModels/ExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels {
    ExposureSettingsViewModel::ExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    ExposureSettingsViewModel::~ExposureSettingsViewModel() {
    }

    void ExposureSettingsViewModel::loadConnections(void) {
    }

    void ExposureSettingsViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels