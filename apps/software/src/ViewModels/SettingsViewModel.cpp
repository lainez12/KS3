#include <ViewModels/SettingsViewModel.h>

namespace Kub3::UI::ViewModels
{
    SettingsViewModel::SettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    SettingsViewModel::~SettingsViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels
