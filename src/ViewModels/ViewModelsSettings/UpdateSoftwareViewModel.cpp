#include <ViewModels/ViewModelsSettings/UpdateSoftwareViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsSettings
{
    UpdateSoftwareViewModel::UpdateSoftwareViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    UpdateSoftwareViewModel::~UpdateSoftwareViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsSettings