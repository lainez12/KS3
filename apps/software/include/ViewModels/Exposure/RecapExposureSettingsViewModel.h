#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseViewModel.h>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class RecapExposureSettingsViewModel final : public ExposureBaseViewModel
    {
        Q_OBJECT
    public:
        explicit RecapExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~RecapExposureSettingsViewModel() override = default;

    public:
        bool isPresetSetAndValid() const;
        PresetExposure getCurrentPreset() const;

    public slots:
        void ps_setExposurePreset(const PresetExposure &preset);

    private:
        PresetExposure m_currentPreset;
        bool m_isPresetSet   = false;
        bool m_isPresetValid = false;
    };

} // namespace Kub3::UI::ViewModels::Exposure
