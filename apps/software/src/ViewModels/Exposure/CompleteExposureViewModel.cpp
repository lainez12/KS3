#include <ViewModels/Exposure/CompleteExposureViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    CompleteExposureViewModel::CompleteExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }
    CompleteExposureViewModel::~CompleteExposureViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Exposure