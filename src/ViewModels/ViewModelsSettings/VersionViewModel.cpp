#include <ViewModels/ViewModelsSettings/VersionViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsSettings
{
    VersionViewModel::VersionViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    VersionViewModel::~VersionViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsSettings