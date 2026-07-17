#include <ViewModels/Settings/AdminPasswordViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{

    AdminPasswordViewModel::AdminPasswordViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    AdminPasswordViewModel::~AdminPasswordViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Alignment