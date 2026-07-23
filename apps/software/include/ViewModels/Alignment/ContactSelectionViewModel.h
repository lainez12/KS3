#ifndef CONTACTSELECTIONVIEWMODEL_H
#define CONTACTSELECTIONVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class ContactSelectionViewModel final : public BaseViewModel
    {
    public:
        explicit ContactSelectionViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ContactSelectionViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Alignment

#endif