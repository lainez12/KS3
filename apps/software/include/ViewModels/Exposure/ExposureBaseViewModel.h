#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QJsonArray>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class ExposureBaseViewModel : public QObject, public BaseViewModel
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

        struct PresetExposure {
            QString name;
            ExposureMode mode = ExposureMode::Continuous;
            ContinuousSettings continuous;
            FlashingSettings flashing;
        };

    public:
        explicit ExposureBaseViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        virtual ~ExposureBaseViewModel() = default;

    protected:
        bool validatePreset(const PresetExposure &preset, QString *errorMessage);
        static QString storagePath();
        static bool ensureParentDirectory(const QString &path, QString *errorMessage);
        static bool loadPresetsFromFile(const QString &path, QJsonArray *presetsArray, QString *errorMessage);
        static bool presetExistsInFile(const QJsonArray &presetsArray, const QString &presetName);
        static bool savePresetsToFile(const QString &path, const QJsonArray &presetsArray, QString *errorMessage);
        static void replaceExistingPreset(QJsonArray &presetsArray, const PresetExposure &preset);


        static QJsonObject presetToJson(const PresetExposure &preset);

        static PresetExposure jsonToPreset(const QJsonObject &json, QString *errorMessage);

    protected:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

    private:
        static QString modeToString(ExposureMode mode);
        static ExposureMode stringToMode(const QString &modeString);
        static QJsonObject durationToJson(const Duration &duration);
        static Duration jsonToDuration(const QJsonObject &json, QString *errorMessage);
    };

} // namespace Kub3::UI::ViewModels::Exposure

using PresetExposure = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::PresetExposure;
using ExposureMode   = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::ExposureMode;
using Duration       = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::Duration;