#include <ViewModels/Settings/OperatingTimesViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    OperatingTimesViewModel::OperatingTimesViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }
    OperatingTimesViewModel::~OperatingTimesViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Settings