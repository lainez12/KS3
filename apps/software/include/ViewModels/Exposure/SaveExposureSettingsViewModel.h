#ifndef SAVEEXPOSURESETTINGSVIEWMODEL_H
#define SAVEEXPOSURESETTINGSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QJsonObject>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

#include <QString>

namespace Kub3::UI::ViewModels::Exposure
{
    class SaveExposureSettingsViewModel final : public BaseViewModel
    {
        Q_OBJECT

    public:
        enum class ExposureMode
        {
            Continuous,
            Flashing,
        };

        struct Duration {
            int minutes = 0;
            int seconds = 0;
        };

        struct ContinuousSettings {
            Duration duration;
            int power = 0;
        };

        struct FlashingSettings {
            int numberOfCycles = 0;
            Duration durationOn;
            Duration durationOff;
            int power = 0;
        };

        struct Preset {
            QString name;
            ExposureMode mode = ExposureMode::Continuous;
            ContinuousSettings continuous;
            FlashingSettings flashing;
        };

    public:
        explicit SaveExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveExposureSettingsViewModel() override;

        bool savePreset(const Preset &preset, QString *errorMessage = nullptr);

    private:
        static QString storagePath();
        static QString modeToString(ExposureMode mode);
        static QJsonObject durationToJson(const Duration &duration);
        static QJsonObject presetToJson(const Preset &preset);
        static bool validatePreset(const Preset &preset, QString *errorMessage);
        static bool ensureParentDirectory(const QString &path, QString *errorMessage);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels

#endif