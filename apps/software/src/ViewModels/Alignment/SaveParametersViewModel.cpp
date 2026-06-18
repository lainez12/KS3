#include <ViewModels/Alignment/SaveParametersViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{

    SaveParametersViewModel::SaveParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    SaveParametersViewModel::~SaveParametersViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Alignment