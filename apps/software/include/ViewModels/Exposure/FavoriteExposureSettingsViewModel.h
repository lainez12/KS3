#ifndef FAVORITEEXPOSURESETTINGSVIEWMODEL_H
#define FAVORITEEXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseViewModel.h>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class FavoriteExposureSettingsViewModel final : public ExposureBaseViewModel
    {
    public:
        explicit FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~FavoriteExposureSettingsViewModel() override;

    public slots:
        void ps_saveExposureSettings(const PresetExposure &settings);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Exposure

#endif