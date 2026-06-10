#ifndef EXPOSURESETTINGSVIEWMODEL_H
#define EXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure
{
    class ExposureSettingsViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit ExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ExposureSettingsViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewModelsExposure

#endif