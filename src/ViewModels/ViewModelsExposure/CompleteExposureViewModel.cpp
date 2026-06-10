#include <ViewModels/ViewModelsExposure/CompleteExposureViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure
{
    CompleteExposureViewModel::CompleteExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    CompleteExposureViewModel::~CompleteExposureViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsExposure