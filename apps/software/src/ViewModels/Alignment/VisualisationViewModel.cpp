#include <ViewModels/Alignment/VisualisationViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{

    VisualisationViewModel::VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    VisualisationViewModel::~VisualisationViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Alignment