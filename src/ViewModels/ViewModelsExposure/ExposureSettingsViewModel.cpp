#include <ViewModels/ViewModelsExposure/ExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure
{
    ExposureSettingsViewModel::ExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    ExposureSettingsViewModel::~ExposureSettingsViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsExposure