#include <ViewModels/Settings/VersionViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    VersionViewModel::VersionViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }
    VersionViewModel::~VersionViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Settings