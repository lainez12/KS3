#ifndef TEMPERATUREVIEWMODEL_H
#define TEMPERATUREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsSettings
{
    class TemperatureViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit TemperatureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~TemperatureViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewModelsSettings

#endif