#ifndef LOADPARAMETERSVIEWMODEL_H
#define LOADPARAMETERSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class LoadParametersViewModel final : public BaseViewModel
    {

    public:
        explicit LoadParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~LoadParametersViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Alignment

#endif