#include <ViewModels/ViewModelsExposure/ProgressExposureViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure
{
    ProgressExposureViewModel::ProgressExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    ProgressExposureViewModel::~ProgressExposureViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsExposure