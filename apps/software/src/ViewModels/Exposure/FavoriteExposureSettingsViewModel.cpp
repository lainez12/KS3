#include <ViewModels/Exposure/FavoriteExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    FavoriteExposureSettingsViewModel::FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }
    FavoriteExposureSettingsViewModel::~FavoriteExposureSettingsViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Exposure