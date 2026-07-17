#ifndef ADMINPASSWORDVIEWMODEL_H
#define ADMINPASSWORDVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    class AdminPasswordViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit AdminPasswordViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~AdminPasswordViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Settings

#endif