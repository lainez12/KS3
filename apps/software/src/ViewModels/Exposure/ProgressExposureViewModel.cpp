#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <ViewModels/Exposure/ProgressExposureViewModel.h>
#include <optional>

namespace Kub3::UI::ViewModels::Exposure
{
    ProgressExposureViewModel::ProgressExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }

    ProgressExposureViewModel::~ProgressExposureViewModel()
    {
    }

    void ProgressExposureViewModel::ui_launchExposure()
    {
        if (m_currentPreset.mode != ExposureMode::Continuous && m_currentPreset.mode != ExposureMode::Flashing)
        {
            qCritical() << "[ProgressExposureViewModel] Invalid exposure mode.";
            emit s_createPopUpWithText("Error launching exposure", {{"OK", [this]() {
                                                                         emit s_closePopUp();
                                                                     }}},
                                       "Invalid exposure mode.");
            return;
        }
        launchExposureToMasterFSM();
    }

    void ProgressExposureViewModel::ps_launchExposure(const PresetExposure &preset)
    {
        m_currentPreset = preset;
    }

    PresetExposure ProgressExposureViewModel::getCurrentPreset() const
    {
        return m_currentPreset;
    }

    double ProgressExposureViewModel::getTemperature() const
    {
        double tempInt = HAL::MS::tryReadDouble(m_repo, INTERNAL_TEMPERATURE).value_or(0.0);
        double tempExt = HAL::MS::tryReadDouble(m_repo, EXTERNAL_TEMPERATURE).value_or(0.0);
        return (tempInt + tempExt) / 2.0; // Average of internal and external temperatures??
    }

    void ProgressExposureViewModel::launchExposureToMasterFSM()
    {
        Services::ExposurePayload payload;

        if (m_currentPreset.mode == ExposureMode::Continuous)
        {
            uint32_t duration   = m_currentPreset.continuous.duration.minutes * 60 * 1000 + m_currentPreset.continuous.duration.milliseconds;
            uint8_t centerPower = m_currentPreset.continuous.power * 0.5;
            uint8_t crownPower  = m_currentPreset.continuous.power * 0.5;
            payload             = HAL::Act::ContinuousExposureParams{
                .durationMs     = duration,
                .centerPowerPct = centerPower,
                .crownPowerPct  = crownPower};
        }
        else if (m_currentPreset.mode == ExposureMode::Flashing)
        {
            uint32_t duration   = m_currentPreset.flashing.durationOn.minutes * 60 * 1000 + m_currentPreset.flashing.durationOn.milliseconds;
            uint32_t pauseTime  = m_currentPreset.flashing.durationOff.minutes * 60 * 1000 + m_currentPreset.flashing.durationOff.milliseconds;
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
    }

} // namespace Kub3::UI::ViewModels::Exposure