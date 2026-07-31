#ifndef CONTACTSELECTIONVIEWMODEL_H
#define CONTACTSELECTIONVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class ContactSelectionViewModel final : public BaseViewModel
    {
        Q_OBJECT
    public:
        enum class ContactMode
        {
            Soft,
            Hard,
        };
    public:
        explicit ContactSelectionViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ContactSelectionViewModel() override;

    public:
        void ui_setSelectedContactMode(ContactMode mode);

    signals:
        void s_contactModeChanged(ContactMode mode);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Alignment

#endif