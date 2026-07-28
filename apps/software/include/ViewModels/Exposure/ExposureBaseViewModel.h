#pragma once

#include <QJsonArray>
#include <QObject>

#include <Common/Result.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
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
            uint32_t minutes;
            uint32_t miliseconds;
        };

        struct ContinuousSettings {
            Duration duration;
            uint8_t power;
        };

        struct FlashingSettings {
            uint32_t numberOfCycles;
            Duration durationOn;
            Duration durationOff;
            uint8_t power;
        };

        struct PresetExposure {
            QString name;
            ExposureMode mode = ExposureMode::Continuous;
            ContinuousSettings continuous;
            FlashingSettings flashing;

            uint32_t getDurationInMS() const
            {
                if (mode == ExposureMode::Continuous)
                {
                    return (continuous.duration.minutes * 60000 + continuous.duration.miliseconds);
                }
                else if (mode == ExposureMode::Flashing)
                {
                    return flashing.numberOfCycles * ((flashing.durationOn.minutes * 60000 + flashing.durationOn.miliseconds) + (flashing.durationOff.minutes * 60000 + flashing.durationOff.miliseconds));
                }
                return 0;
            }
        };

    public:
        explicit ExposureBaseViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        virtual ~ExposureBaseViewModel() = default;

    public:
        static QString presetDetailsToStr(const PresetExposure &preset);
        static QString modeToString(ExposureMode mode);
        Result<Unit, const char *> validatePreset(const PresetExposure &preset);

    protected:
        static QString storagePath();
        static Result<Unit, QString> ensureParentDirectory(const QString &path);
        static Result<Unit, QString> loadPresetsFromFile(const QString &path, QJsonArray *presetsArray);
        static bool presetExistsInFile(const QJsonArray &presetsArray, const QString &presetName);
        static Result<Unit, QString> savePresetsToFile(const QString &path, const QJsonArray &presetsArray);
        static void replaceExistingPreset(QJsonArray &presetsArray, const PresetExposure &preset);

        static Result<PresetExposure, QString> getPresetByName(const QJsonArray &presetsArray, const QString &presetName);
        static bool deleteByName(QJsonArray &presetsArray, const QString &presetName);

        static QJsonObject presetToJson(const PresetExposure &preset);

        static Result<PresetExposure, QString> jsonToPreset(const QJsonObject json);

    protected:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

    private:
        static ExposureMode stringToMode(const QString &modeString);
        static QJsonObject durationToJson(const Duration &duration);
        static Result<Duration, QString> jsonToDuration(const QJsonObject &json);
    };

} // namespace Kub3::UI::ViewModels::Exposure

using PresetExposure = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::PresetExposure;
using ExposureMode   = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::ExposureMode;
using Duration       = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::Duration;