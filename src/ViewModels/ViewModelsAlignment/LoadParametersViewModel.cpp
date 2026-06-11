#include <ViewModels/ViewModelsAlignment/LoadParametersViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    LoadParametersViewModel::LoadParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    LoadParametersViewModel::~LoadParametersViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Alignment