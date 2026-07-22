#ifndef RECAPEXPOSURESETTINGSVIEWMODEL_H
#define RECAPEXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseViewModel.h>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class RecapExposureSettingsViewModel final : public ExposureBaseViewModel
    {
    public:
        explicit RecapExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~RecapExposureSettingsViewModel() override;

    public slots:
        void ps_setExposurePreset(const PresetExposure &preset);
        PresetExposure getCurrentPreset() const;
        bool isPresetSetAndValid() const;
        bool isFlashingMode() const;

    private:
        PresetExposure m_currentPreset;
        bool m_isPresetSet    = false;
        bool m_isPresetValid  = false;
        bool m_isFlashingMode = false;
    };

} // namespace Kub3::UI::ViewModels

#endif