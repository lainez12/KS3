#ifndef FAVORITEEXPOSURESETTINGSVIEWMODEL_H
#define FAVORITEEXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class FavoriteExposureSettingsViewModel final : public BaseViewModel
    {
    public:
        explicit FavoriteExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~FavoriteExposureSettingsViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Exposure

#endif