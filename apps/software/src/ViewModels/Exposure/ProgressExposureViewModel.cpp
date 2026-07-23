#include <ViewModels/Exposure/ProgressExposureViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    ProgressExposureViewModel::ProgressExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    ProgressExposureViewModel::~ProgressExposureViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Exposure