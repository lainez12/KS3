#include <ViewModels/Settings/AdminPasswordViewModel.h>
#include <system.h>

namespace Kub3::UI::ViewModels::Settings
{

    AdminPasswordViewModel::AdminPasswordViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    AdminPasswordViewModel::~AdminPasswordViewModel()
    {
    }

    void AdminPasswordViewModel::submitPassword(const QString &inputPasswd)
    {
        const bool ok = verifyRootPassword(inputPasswd.toStdString());

        if (ok)
            emit s_authenticationSuccess();
        else
            emit s_authenticationFailure();
    }

} // namespace Kub3::UI::ViewModels::Settings
