#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTextStream>
#include <ViewModels/Exposure/ExposureBaseViewModel.h>

namespace
{
    constexpr auto kRootKey       = "presets";
    constexpr auto kVersionKey    = "version";
    constexpr auto kNameKey       = "name";
    constexpr auto kModeKey       = "mode";
    constexpr auto kContinuousKey = "Continuous";
    constexpr auto kFlashingKey   = "Flashing";
}

namespace Kub3::UI::ViewModels::Exposure
{
    ExposureBaseViewModel::ExposureBaseViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    bool ExposureBaseViewModel::validatePreset(const PresetExposure &preset, QString *errorMessage)
    {
        if (preset.name.trimmed().isEmpty())
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("The preset name cannot be empty.");
            return false;
        }

        const auto validateDuration = [errorMessage](const Duration &duration, const QString &label) {
            bool zeroDuration = (duration.minutes == 0 && duration.seconds == 0);
            if (duration.minutes < 0 || duration.seconds < 0 || duration.seconds >= 60 || zeroDuration)
            {
                if (errorMessage)
                    *errorMessage = QStringLiteral("The duration %1 must have minutes >= 0 and seconds between 0 and 59.").arg(label);
                return false;
            }

            return true;
        };

        if (preset.mode == ExposureMode::Continuous)
        {
            if (!validateDuration(preset.continuous.duration, QStringLiteral("Continuous")))
                return false;
            if (preset.continuous.power < 0)
            {
                if (errorMessage)
                    *errorMessage = QStringLiteral("The continuous power cannot be negative.");
                return false;
            }
        }
        else
        {
            if (preset.flashing.numberOfCycles < 0)
            {
                if (errorMessage)
                    *errorMessage = QStringLiteral("The number of cycles cannot be negative.");
                return false;
            }
            if (!validateDuration(preset.flashing.durationOn, QStringLiteral("on")))
                return false;
            if (!validateDuration(preset.flashing.durationOff, QStringLiteral("off")))
                return false;
            if (preset.flashing.power < 0)
            {
                if (errorMessage)
                    *errorMessage = QStringLiteral("The flashing power cannot be negative.");
                return false;
            }
        }

        return true;
    }

    QString ExposureBaseViewModel::storagePath()
    {
        return QStringLiteral(KUB3_SAVES_EXPOSURE_SETTINGS_PATH);
    }

    bool ExposureBaseViewModel::ensureParentDirectory(const QString &path, QString *errorMessage)
    {
        const QFileInfo fileInfo(path);
        const QDir directory = fileInfo.dir();
        if (directory.exists())
            return true;

        if (QDir().mkpath(directory.absolutePath()))
            return true;

        if (errorMessage)
            *errorMessage = QStringLiteral("The destination directory could not be created: %1").arg(directory.absolutePath());
        return false;
    }

    bool ExposureBaseViewModel::loadPresetsFromFile(const QString &path, QJsonArray *presetsArray, QString *errorMessage)
    {

        QFile inputFile(path);
        if (inputFile.exists())
        {
            if (!inputFile.open(QIODevice::ReadOnly))
            {
                if (errorMessage)
                    *errorMessage = QStringLiteral("The preset file could not be opened for reading.: %1").arg(inputFile.errorString());
                return false;
            }

            const QByteArray rawData = inputFile.readAll();
            inputFile.close();

            if (!rawData.isEmpty())
            {
                QJsonParseError parseError{};
                const QJsonDocument document = QJsonDocument::fromJson(rawData, &parseError);
                if (parseError.error != QJsonParseError::NoError)
                {
                    if (errorMessage)
                        *errorMessage = QStringLiteral("The preset file has an invalid JSON format: %1").arg(parseError.errorString());
                    return false;
                }

                if (document.isArray())
                {
                    *presetsArray = document.array();
                }
                else if (document.isObject())
                {
                    const QJsonObject rootObject = document.object();
                    *presetsArray                = rootObject.value(QLatin1String(kRootKey)).toArray();
                }
            }
        }
        return true;
    }

    bool ExposureBaseViewModel::presetExistsInFile(const QJsonArray &presetsArray, const QString &presetName)
    {
        for (const QJsonValue &value : presetsArray)
        {
            const QJsonObject presetObject = value.toObject();
            if (presetObject.value(QLatin1String(kNameKey)).toString() == presetName)
                return true;
        }
        return false;
    }

    void ExposureBaseViewModel::replaceExistingPreset(QJsonArray &presetsArray, const PresetExposure &preset)
    {
        for (int i = 0; i < presetsArray.size(); ++i)
        {
            QJsonObject presetObject = presetsArray[i].toObject();
            if (presetObject.value(QLatin1String(kNameKey)).toString() == preset.name)
            {
                presetsArray[i] = presetToJson(preset);
                break;
            }
        }
    }

    PresetExposure ExposureBaseViewModel::getPresetByName(const QJsonArray &presetsArray, const QString &presetName, QString *errorMessage)
    {
        for (const QJsonValue &value : presetsArray)
        {
            const QJsonObject presetObject = value.toObject();
            if (presetObject.value(QLatin1String(kNameKey)).toString() == presetName)
            {
                PresetExposure preset = jsonToPreset(presetObject, errorMessage);
                if (!preset.name.isEmpty() && (errorMessage == nullptr || errorMessage->isEmpty()))
                    return preset;
                else
                {
                    qDebug() << "Error parsing preset: " << *errorMessage;
                    return {};
                }
            }
        }

        if (errorMessage)
            *errorMessage = QStringLiteral("The preset with name '%1' was not found.").arg(presetName);
        return {};
    }

    bool ExposureBaseViewModel::savePresetsToFile(const QString &path, const QJsonArray &presetsArray, QString *errorMessage)
    {
        QJsonObject rootObject;
        rootObject.insert(QLatin1String(kVersionKey), 1);
        rootObject.insert(QLatin1String(kRootKey), presetsArray);

        QSaveFile outputFile(path);
        if (!outputFile.open(QIODevice::WriteOnly))
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("The preset file could not be opened for writing: %1").arg(outputFile.errorString());
            return false;
        }

        outputFile.write(QJsonDocument(rootObject).toJson(QJsonDocument::Indented));
        if (!outputFile.commit())
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("The preset file could not be saved: %1").arg(outputFile.errorString());
            return false;
        }

        return true;
    }

    QJsonObject ExposureBaseViewModel::presetToJson(const PresetExposure &preset)
    {
        QJsonObject json;
        json.insert(QLatin1String(kNameKey), preset.name);
        json.insert(QLatin1String(kModeKey), modeToString(preset.mode));

        if (preset.mode == ExposureMode::Continuous)
        {
            QJsonObject continuousObject;
            continuousObject.insert(QStringLiteral("duration"), durationToJson(preset.continuous.duration));
            continuousObject.insert(QStringLiteral("power"), preset.continuous.power);
            json.insert(QLatin1String(kContinuousKey), continuousObject);
        }
        else
        {
            QJsonObject flashingObject;
            flashingObject.insert(QStringLiteral("numberOfCycles"), preset.flashing.numberOfCycles);
            flashingObject.insert(QStringLiteral("durationOn"), durationToJson(preset.flashing.durationOn));
            flashingObject.insert(QStringLiteral("durationOff"), durationToJson(preset.flashing.durationOff));
            flashingObject.insert(QStringLiteral("power"), preset.flashing.power);
            json.insert(QLatin1String(kFlashingKey), flashingObject);
        }

        return json;
    }

    QString ExposureBaseViewModel::modeToString(ExposureMode mode)
    {
        switch (mode)
        {
        case ExposureMode::Continuous:
            return QStringLiteral("Continuous");
        case ExposureMode::Flashing:
            return QStringLiteral("Flashing");
        }

        return QStringLiteral("Continuous");
    }

    QJsonObject ExposureBaseViewModel::durationToJson(const Duration &duration)
    {
        QJsonObject json;
        json.insert(QStringLiteral("minutes"), duration.minutes);
        json.insert(QStringLiteral("seconds"), duration.seconds);
        return json;
    }

    PresetExposure ExposureBaseViewModel::jsonToPreset(const QJsonObject json, QString *errorMessage)
    {
        PresetExposure preset;

        if (json.contains(QLatin1String(kNameKey)))
            preset.name = json.value(QLatin1String(kNameKey)).toString();
        else
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("The preset JSON object is missing the 'name' field.");
            return {};
        }

        if (json.contains(QLatin1String(kModeKey)))
        {
            const QString modeStr = json.value(QLatin1String(kModeKey)).toString();
            preset.mode           = stringToMode(modeStr);
        }
        else
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("The preset JSON object is missing the 'mode' field.");
            return {};
        }

        if (preset.mode == ExposureMode::Continuous)
        {
            if (json.contains(QLatin1String(kContinuousKey)))
            {
                const QJsonObject continuousJson = json.value(QLatin1String(kContinuousKey)).toObject();
                preset.continuous.duration       = jsonToDuration(continuousJson.value(QLatin1String("duration")).toObject(), errorMessage);
                preset.continuous.power          = continuousJson.value(QStringLiteral("power")).toInt();
            }
        }
        else
        {
            if (json.contains(QLatin1String(kFlashingKey)))
            {
                const QJsonObject flashingJson = json.value(QLatin1String(kFlashingKey)).toObject();
                preset.flashing.numberOfCycles = flashingJson.value(QStringLiteral("numberOfCycles")).toInt();
                preset.flashing.durationOn     = jsonToDuration(flashingJson.value(QLatin1String("durationOn")).toObject(), errorMessage);
                preset.flashing.durationOff    = jsonToDuration(flashingJson.value(QLatin1String("durationOff")).toObject(), errorMessage);
                preset.flashing.power          = flashingJson.value(QStringLiteral("power")).toInt();
            }
        }

        return preset;
    }

    QString ExposureBaseViewModel::presetDetailsToStr(const PresetExposure &preset)
    {
        if (preset.mode == ExposureMode::Continuous)
        {
            return QStringLiteral("Exposure duration: %1min %2s\nExposure power: %3%")
                .arg(preset.continuous.duration.minutes)
                .arg(preset.continuous.duration.seconds)
                .arg(preset.continuous.power);
        }
        else
        {
            return QStringLiteral("Number of cycles: %1\nDuration Ton: %2min %3s\nDuration Toff: %4min %5s\nExposure power: %6%")
                .arg(preset.flashing.numberOfCycles)
                .arg(preset.flashing.durationOn.minutes)
                .arg(preset.flashing.durationOn.seconds)
                .arg(preset.flashing.durationOff.minutes)
                .arg(preset.flashing.durationOff.seconds)
                .arg(preset.flashing.power);
        }
    }

    ExposureMode ExposureBaseViewModel::stringToMode(const QString &modeString)
    {
        if (modeString == QStringLiteral("Continuous"))
            return ExposureMode::Continuous;
        else if (modeString == QStringLiteral("Flashing"))
            return ExposureMode::Flashing;

        // Default to Continuous if the string is unrecognized
        return ExposureMode::Continuous;
    }

    Duration ExposureBaseViewModel::jsonToDuration(const QJsonObject &json, QString *errorMessage)
    {
        Duration duration;

        if (json.contains(QStringLiteral("minutes")))
            duration.minutes = json.value(QStringLiteral("minutes")).toInt();
        else
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("The duration JSON object is missing the 'minutes' field.");
            return {};
        }

        if (json.contains(QStringLiteral("seconds")))
            duration.seconds = json.value(QStringLiteral("seconds")).toInt();
        else
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("The duration JSON object is missing the 'seconds' field.");
            return {};
        }

        return duration;
    }
} // namespace Kub3::UI::ViewModels::Exposure