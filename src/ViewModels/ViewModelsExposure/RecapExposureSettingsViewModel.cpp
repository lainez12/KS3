#include <ViewModels/ViewModelsExposure/RecapExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure
{
    RecapExposureSettingsViewModel::RecapExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    RecapExposureSettingsViewModel::~RecapExposureSettingsViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsExposure