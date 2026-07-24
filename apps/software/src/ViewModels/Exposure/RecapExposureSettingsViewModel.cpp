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

        Services::ExposurePayload payload;

        if (m_currentPreset.mode == ExposureMode::Continuous)
        {
            uint32_t duration   = m_currentPreset.continuous.duration.minutes * 60 * 1000 + m_currentPreset.continuous.duration.miliseconds; // TODO seconds should be a double to allow for sub-second precision
            uint8_t centerPower = m_currentPreset.continuous.power * 0.5;                                                                    // TODO
            uint8_t crownPower  = m_currentPreset.continuous.power * 0.5;                                                                    // TODO
            payload             = HAL::Act::ContinuousExposureParams{
                .durationMs     = duration,
                .centerPowerPct = centerPower,
                .crownPowerPct  = crownPower};
        }
        else if (m_currentPreset.mode == ExposureMode::Flashing)
        {
            uint32_t duration   = m_currentPreset.flashing.durationOn.minutes * 60 * 1000 + m_currentPreset.flashing.durationOn.miliseconds;
            uint32_t pauseTime  = m_currentPreset.flashing.durationOff.minutes * 60 * 1000 + m_currentPreset.flashing.durationOff.miliseconds;
            uint8_t centerPower = m_currentPreset.flashing.power * 0.5; // TODO
            uint8_t crownPower  = m_currentPreset.flashing.power * 0.5; // TODO
            payload             = HAL::Act::FlashingExposureParams{
                .cycles         = m_currentPreset.flashing.numberOfCycles,
                .durationMs     = duration,
                .pauseTimeMs    = pauseTime,
                .centerPowerPct = centerPower,
                .crownPowerPct  = crownPower};
        }
        else
        {
            qCritical() << "[RecapExposureSettingsViewModel] Unknown exposure mode.";
            return;
        }

        emit s_launchExposure(payload);
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
