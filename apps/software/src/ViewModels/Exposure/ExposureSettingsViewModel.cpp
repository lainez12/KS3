#include <ViewModels/Exposure/ExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    ExposureSettingsViewModel::ExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }
    ExposureSettingsViewModel::~ExposureSettingsViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Exposure