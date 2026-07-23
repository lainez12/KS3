#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QJsonArray>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Exposure
{
    class ExposureBaseViewModel : public BaseViewModel
    {
        Q_OBJECT

    public:
        enum class ExposureMode
        {
            Continuous,
            Flashing,
        };

        struct Duration {
            int minutes;
            int seconds;
        };

        struct ContinuousSettings {
            Duration duration;
            int power;
        };

        struct FlashingSettings {
            int numberOfCycles;
            Duration durationOn;
            Duration durationOff;
            int power;
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

    public:
        static QString presetDetailsToStr(const PresetExposure &preset);
        static QString modeToString(ExposureMode mode);
        bool validatePreset(const PresetExposure &preset, QString *errorMessage);

    protected:
        static QString storagePath();
        static bool ensureParentDirectory(const QString &path, QString *errorMessage);
        static bool loadPresetsFromFile(const QString &path, QJsonArray *presetsArray, QString *errorMessage);
        static bool presetExistsInFile(const QJsonArray &presetsArray, const QString &presetName);
        static bool savePresetsToFile(const QString &path, const QJsonArray &presetsArray, QString *errorMessage);
        static void replaceExistingPreset(QJsonArray &presetsArray, const PresetExposure &preset);

        static PresetExposure getPresetByName(const QJsonArray &presetsArray, const QString &presetName, QString *errorMessage);

        static QJsonObject presetToJson(const PresetExposure &preset);

        static PresetExposure jsonToPreset(const QJsonObject json, QString *errorMessage);

    protected:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

    private:
        static ExposureMode stringToMode(const QString &modeString);
        static QJsonObject durationToJson(const Duration &duration);
        static Duration jsonToDuration(const QJsonObject &json, QString *errorMessage);
    };

} // namespace Kub3::UI::ViewModels::Exposure

using PresetExposure = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::PresetExposure;
using ExposureMode   = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::ExposureMode;
using Duration       = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::Duration;