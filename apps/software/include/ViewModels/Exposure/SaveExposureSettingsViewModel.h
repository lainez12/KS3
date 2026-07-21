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

        bool savePreset(const PresetExposure &preset, QString *errorMessage = nullptr);

    public slots:
        void ps_saveExposureSettings(const PresetExposure &settings);
    
    private:
        static QString storagePath();
        static QString modeToString(ExposureMode mode);
        static QJsonObject durationToJson(const Duration &duration);
        static QJsonObject presetToJson(const PresetExposure &preset);
        static bool validatePreset(const PresetExposure &preset, QString *errorMessage);
        static bool ensureParentDirectory(const QString &path, QString *errorMessage);
    };

} // namespace Kub3::UI::ViewModels

#endif