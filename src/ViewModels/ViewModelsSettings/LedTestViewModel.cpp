#include <ViewModels/ViewModelsSettings/LedTestViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    LedTestViewModel::LedTestViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    LedTestViewModel::~LedTestViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Settings