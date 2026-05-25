#include <ViewModels/FavoriteExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels {
    FavoriteExposureSettingsViewModel::FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    FavoriteExposureSettingsViewModel::~FavoriteExposureSettingsViewModel() {
    }

    void FavoriteExposureSettingsViewModel::loadConnections(void) {
    }

    void FavoriteExposureSettingsViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels