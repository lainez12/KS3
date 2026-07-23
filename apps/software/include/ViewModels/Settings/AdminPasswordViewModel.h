#ifndef ADMINPASSWORDVIEWMODEL_H
#define ADMINPASSWORDVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    class AdminPasswordViewModel final : public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit AdminPasswordViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~AdminPasswordViewModel() override;

    public:
        void submitPassword(const QString &inputPasswd);

    signals:
        void s_authenticationSuccess(void);
        void s_authenticationFailure(void);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Settings

#endif