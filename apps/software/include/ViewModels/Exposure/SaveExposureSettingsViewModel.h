#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>

#include <Common/Result.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/BaseViewModel.h>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class SaveExposureSettingsViewModel final : public ExposureBaseViewModel
    {
        Q_OBJECT

    public:
        explicit SaveExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveExposureSettingsViewModel() override = default;

        Result<Unit, QString> savePreset(const PresetExposure &preset, bool replaceExisting = true);
        void userConfirmSavePreset(const QString &name);

        Result<QList<PresetExposure>, QString> getAllPresets();

    signals:
        void s_presetSaved();
        void s_errorSavingPreset(const QString &errorMessage);

    public slots:
        void ps_saveExposureSettings(const PresetExposure &settings);

    private:
        PresetExposure m_currentPreset;
    };

} // namespace Kub3::UI::ViewModels::Exposure
