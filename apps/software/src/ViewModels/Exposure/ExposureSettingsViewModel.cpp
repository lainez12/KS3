#include <ViewModels/Exposure/ExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    ExposureSettingsViewModel::ExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }
    ExposureSettingsViewModel::~ExposureSettingsViewModel()
    {
    }

    void ExposureSettingsViewModel::ui_requestSaveExposureSettings(const PresetExposure &settings)
    {
        emit s_requestSaveExposureSettings(settings);
    }

} // namespace Kub3::UI::ViewModels::Exposure