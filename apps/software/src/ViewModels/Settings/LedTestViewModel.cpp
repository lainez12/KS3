#include <ViewModels/Settings/LedTestViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    LedTestViewModel::LedTestViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }
    LedTestViewModel::~LedTestViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Settings