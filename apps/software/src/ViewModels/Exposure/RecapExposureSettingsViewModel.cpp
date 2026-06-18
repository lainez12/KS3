#include <ViewModels/Exposure/RecapExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    RecapExposureSettingsViewModel::RecapExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    RecapExposureSettingsViewModel::~RecapExposureSettingsViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Exposure