#include <ViewModels/Exposure/FavoriteExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    FavoriteExposureSettingsViewModel::FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureSettingsViewModelBase(repo, parent)
    {
    }
    FavoriteExposureSettingsViewModel::~FavoriteExposureSettingsViewModel()
    {
    }

    void FavoriteExposureSettingsViewModel::ps_saveExposureSettings(const PresetExposure &settings)
    {
        qDebug() << "[FavoriteExposureSettingsViewModel::ps_saveExposureSettings]: Saving exposure settings...";
        QString modeStr = (settings.mode == ExposureMode::Continuous) ? "Continuous" : "Flashing";
        qDebug() << "[FavoriteExposureSettingsViewModel::ps_saveExposureSettings]: Mode: " << modeStr;
    }

} // namespace Kub3::UI::ViewModels::Exposure