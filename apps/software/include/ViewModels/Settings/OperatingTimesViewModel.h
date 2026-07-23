#ifndef OPERATINGTIMESVIEWMODEL_H
#define OPERATINGTIMESVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    class OperatingTimesViewModel final : public BaseViewModel
    {
    public:
        explicit OperatingTimesViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~OperatingTimesViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Settings

#endif