#ifndef SAVEPARAMETERSVIEWMODEL_H
#define SAVEPARAMETERSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class SaveParametersViewModel final : public BaseViewModel
    {

    public:
        explicit SaveParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveParametersViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Alignment

#endif