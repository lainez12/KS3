#include <ViewModels/Exposure/RecapExposureSettingsViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    RecapExposureSettingsViewModel::RecapExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }

    void RecapExposureSettingsViewModel::ps_setExposurePreset(const PresetExposure &preset)
    {
        m_currentPreset = preset;
        m_isPresetSet   = true;
        m_isPresetValid = validatePreset(preset).is_ok();
    }

    PresetExposure RecapExposureSettingsViewModel::getCurrentPreset() const
    {
        return m_currentPreset;
    }

    bool RecapExposureSettingsViewModel::isPresetSetAndValid() const
    {
        return m_isPresetSet && m_isPresetValid;
    }

} // namespace Kub3::UI::ViewModels::Exposure
