#include <ViewModels/ViewModelsExposure/SaveExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure
{
    SaveExposureSettingsViewModel::SaveExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    SaveExposureSettingsViewModel::~SaveExposureSettingsViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsExposure