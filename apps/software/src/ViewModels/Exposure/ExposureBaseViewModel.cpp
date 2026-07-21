#include <ViewModels/Exposure/ExposureBaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    ExposureBaseViewModel::ExposureBaseViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

} // namespace Kub3::UI::ViewModels::Exposure