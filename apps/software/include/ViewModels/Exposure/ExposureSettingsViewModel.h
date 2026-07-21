#ifndef EXPOSURESETTINGSVIEWMODEL_H
#define EXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/Exposure/ExposureSettingsViewModelBase.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class ExposureSettingsViewModel final : public ExposureSettingsViewModelBase
    {
    public:
        explicit ExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ExposureSettingsViewModel() override;

    public:
        void ui_requestSaveExposureSettings(const PresetExposure &settings);

    signals:
        void s_requestSaveExposureSettings(const PresetExposure &settings);

    private:
    };

} // namespace Kub3::UI::ViewModels::Exposure

#endif