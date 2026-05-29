#include <ViewModels/ViewModelsExposure/RecapExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure {
    RecapExposureSettingsViewModel::RecapExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }

    RecapExposureSettingsViewModel::~RecapExposureSettingsViewModel() {
    }

    void RecapExposureSettingsViewModel::loadConnections(void) {
    }

    void RecapExposureSettingsViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsExposure