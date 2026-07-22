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
    public:
        explicit FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~FavoriteExposureSettingsViewModel() override;

        bool getAllExposureSettings(QList<FavoriteExposureSettingButton *> &presetButtons, QString *errorMessage);
        bool ui_userChoseExposurePreset(QString &presetName);

    signals:
        void s_exposurePresetChosen(const PresetExposure &preset);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Exposure

#endif