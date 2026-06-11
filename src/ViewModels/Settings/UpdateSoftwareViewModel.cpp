#include <ViewModels/Settings/UpdateSoftwareViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    UpdateSoftwareViewModel::UpdateSoftwareViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    UpdateSoftwareViewModel::~UpdateSoftwareViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Settings