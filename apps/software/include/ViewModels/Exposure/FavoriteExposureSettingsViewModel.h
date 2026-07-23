#ifndef FAVORITEEXPOSURESETTINGSVIEWMODEL_H
#define FAVORITEEXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseViewModel.h>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>
#include <Views/Components/FavoriteExposureSettingButton.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class FavoriteExposureSettingsViewModel final : public ExposureBaseViewModel
    {
        Q_OBJECT

    public:
        explicit FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);

    public:
        bool getAllExposureSettings(QList<FavoriteExposureSettingButton *> &presetButtons, QString *errorMessage);
        bool uiLoadExposurePreset(QString &presetName);

    signals:
        void s_exposurePresetLoaded(const PresetExposure &preset);
    };

} // namespace Kub3::UI::ViewModels::Exposure

#endif