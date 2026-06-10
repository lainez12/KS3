#ifndef RECAPEXPOSURESETTINGSVIEWMODEL_H
#define RECAPEXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure
{
    class RecapExposureSettingsViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit RecapExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~RecapExposureSettingsViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels

#endif