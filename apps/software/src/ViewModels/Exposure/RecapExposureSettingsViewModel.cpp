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

    void RecapExposureSettingsViewModel::ui_requestLaunchExposure(void)
    {
        if (!m_isPresetSet || !m_isPresetValid)
        {
            qWarning() << "[RecapExposureSettingsViewModel] Attempted to launch exposure with an unset or invalid preset.";
            return;
        }

        
        emit s_exposurePresetLaunched(m_currentPreset);
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
