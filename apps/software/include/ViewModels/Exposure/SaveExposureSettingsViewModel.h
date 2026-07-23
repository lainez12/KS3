#ifndef SAVEEXPOSURESETTINGSVIEWMODEL_H
#define SAVEEXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QJsonObject>
#include <ViewModels/BaseViewModel.h>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>

#include <QString>

namespace Kub3::UI::ViewModels::Exposure
{
    class SaveExposureSettingsViewModel final : public ExposureBaseViewModel
    {
        Q_OBJECT

    public:
        explicit SaveExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveExposureSettingsViewModel() override;

        bool savePreset(const PresetExposure &preset, QString *errorMessage = nullptr, bool replaceExisting = true);
        void userConfirmSavePreset(const QString &name);
        QList<PresetExposure> getAllPresets(QString *errorMessage);

    signals:
        void s_presetSaved();
        void s_errorSavingPreset(const QString &errorMessage);

    public slots:
        void ps_saveExposureSettings(const PresetExposure &settings);

    private:
        PresetExposure m_currentPreset;
        QString m_nameUserInput;
    };

} // namespace Kub3::UI::ViewModels

#endif