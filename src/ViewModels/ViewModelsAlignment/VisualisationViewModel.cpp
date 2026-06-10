#include <ViewModels/ViewModelsAlignment/VisualisationViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsAlignment
{

    VisualisationViewModel::VisualisationViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    VisualisationViewModel::~VisualisationViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsAlignment