#include <ViewModels/ViewModelsSettings/OperatingTimesViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsSettings
{
    OperatingTimesViewModel::OperatingTimesViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    OperatingTimesViewModel::~OperatingTimesViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsSettings