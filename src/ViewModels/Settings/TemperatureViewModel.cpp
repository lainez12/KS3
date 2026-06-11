#include <ViewModels/Settings/TemperatureViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    TemperatureViewModel::TemperatureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    TemperatureViewModel::~TemperatureViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Settings