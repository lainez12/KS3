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

    Result<Unit, const char *> ExposureBaseViewModel::validatePreset(const PresetExposure &preset)
    {
        if (preset.name.trimmed().isEmpty())
        {
            return Err("The preset name cannot be empty.");
        }

        const auto durationCheck = [](const Duration &d) -> bool {
            return (d.minutes > 0 || d.miliseconds > 0) && d.miliseconds < 59999;
        };

        if (preset.mode == ExposureMode::Continuous)
        {
            if (!durationCheck(preset.continuous.duration))
                return Err("Invalid exposure duration.");
            if (preset.continuous.power == 0)
                return Err("Continuous exposure power cannot be zero.");
        }
        else
        {
            if (preset.flashing.numberOfCycles == 0)
                return Err("Flashing number of cycles cannot be 0.");
            if (!durationCheck(preset.flashing.durationOn))
                return Err("Invalid flashing active duration.");
            if (!durationCheck(preset.flashing.durationOff))
                return Err("Invalid flashing inactive duration.");
            if (preset.flashing.power == 0)
                return Err("Flashing exposure power cannot be zero.");
        }

        return Ok<Unit>({});
    }

    QString ExposureBaseViewModel::storagePath()
    {
        return QStringLiteral(KUB3_SAVES_EXPOSURE_SETTINGS_PATH);
    }

    Result<Unit, QString> ExposureBaseViewModel::ensureParentDirectory(const QString &path)
    {
        const QFileInfo fileInfo(path);
        const QDir directory = fileInfo.dir();

        if (directory.exists())
            return Ok<Unit>({});
        if (QDir().mkpath(directory.absolutePath()))
            return Ok<Unit>({});

        return Err(QStringLiteral("The destination directory could not be created: %1").arg(directory.absolutePath()));
    }

    Result<Unit, QString> ExposureBaseViewModel::loadPresetsFromFile(const QString &path, QJsonArray *presetsArray)
    {
        QFile inputFile(path);

        if (inputFile.exists())
        {
            if (!inputFile.open(QIODevice::ReadOnly))
            {
                return Err(QStringLiteral("The preset file could not be opened for reading: %1").arg(inputFile.errorString()));
            }

            const QByteArray rawData = inputFile.readAll();
            inputFile.close();

            if (!rawData.isEmpty())
            {
                QJsonParseError parseError{};
                const QJsonDocument document = QJsonDocument::fromJson(rawData, &parseError);

                if (parseError.error != QJsonParseError::NoError)
                {
                    return Err(QStringLiteral("The preset file has an invalid JSON format: %1").arg(parseError.errorString()));
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
        return Ok<Unit>({});
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

    Result<PresetExposure, QString> ExposureBaseViewModel::getPresetByName(const QJsonArray &presetsArray, const QString &presetName)
    {
        for (const QJsonValue &value : presetsArray)
        {
            const QJsonObject presetObject = value.toObject();

            if (presetObject.value(QLatin1String(kNameKey)).toString() == presetName)
            {
                return jsonToPreset(presetObject);
            }
        }

        return Err(QStringLiteral("The preset with name '%1' was not found.").arg(presetName));
    }

    bool ExposureBaseViewModel::deleteByName(QJsonArray &presetsArray, const QString &presetName)
    {
        for (int i = 0; i < presetsArray.size(); ++i)
        {
            QJsonObject presetObject = presetsArray[i].toObject();
            if (presetObject.value(QLatin1String(kNameKey)).toString() == presetName)
            {
                presetsArray.removeAt(i);
                return true;
            }
        }
        return false;
    }

    Result<Unit, QString> ExposureBaseViewModel::savePresetsToFile(const QString &path, const QJsonArray &presetsArray)
    {
        QJsonObject rootObject;
        rootObject.insert(QLatin1String(kVersionKey), 1);
        rootObject.insert(QLatin1String(kRootKey), presetsArray);

        QSaveFile outputFile(path);
        if (!outputFile.open(QIODevice::WriteOnly))
        {
            return Err(QStringLiteral("The preset file could not be opened for writing: %1").arg(outputFile.errorString()));
        }

        outputFile.write(QJsonDocument(rootObject).toJson(QJsonDocument::Indented));
        if (!outputFile.commit())
        {
            return Err(QStringLiteral("The preset file could not be saved: %1").arg(outputFile.errorString()));
        }

        return Ok<Unit>({});
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
            continuousObject.insert(QStringLiteral("power"), static_cast<int>(preset.continuous.power));
            json.insert(QLatin1String(kContinuousKey), continuousObject);
        }
        else
        {
            QJsonObject flashingObject;
            flashingObject.insert(QStringLiteral("numberOfCycles"), static_cast<int>(preset.flashing.numberOfCycles));
            flashingObject.insert(QStringLiteral("durationOn"), durationToJson(preset.flashing.durationOn));
            flashingObject.insert(QStringLiteral("durationOff"), durationToJson(preset.flashing.durationOff));
            flashingObject.insert(QStringLiteral("power"), static_cast<int>(preset.flashing.power));
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
        json.insert(QStringLiteral("minutes"), static_cast<int>(duration.minutes));
        json.insert(QStringLiteral("miliseconds"), static_cast<int>(duration.miliseconds));
        return json;
    }

    Result<PresetExposure, QString> ExposureBaseViewModel::jsonToPreset(const QJsonObject json)
    {
        PresetExposure preset;

        if (!json.contains(QLatin1String(kNameKey)))
            return Err(QStringLiteral("The preset JSON object is missing the 'name' field."));
        if (!json.contains(QLatin1String(kModeKey)))
            return Err(QStringLiteral("The preset JSON object is missing the 'mode' field."));

        preset.name = json.value(QLatin1String(kNameKey)).toString();
        preset.mode = stringToMode(json.value(QLatin1String(kModeKey)).toString());

        if (preset.mode == ExposureMode::Continuous)
        {
            if (!json.contains(QLatin1String(kContinuousKey)))
                return Err(QStringLiteral("The preset JSON object is missing the 'continuous' field."));

            const QJsonObject continuousJson = json.value(QLatin1String(kContinuousKey)).toObject();

            auto durationRes = jsonToDuration(continuousJson.value(QLatin1String("duration")).toObject());
            if (durationRes.is_err())
                return Err(durationRes.unwrap_err());

            preset.continuous.duration = *durationRes;
            preset.continuous.power    = continuousJson.value(QStringLiteral("power")).toInt();
        }
        else
        {
            if (!json.contains(QLatin1String(kFlashingKey)))
                return Err(QStringLiteral("The preset JSON object is missing the 'flashing' field."));

            const QJsonObject flashingJson = json.value(QLatin1String(kFlashingKey)).toObject();

            preset.flashing.numberOfCycles = flashingJson.value(QStringLiteral("numberOfCycles")).toInt();

            auto durationOnRes = jsonToDuration(flashingJson.value(QLatin1String("durationOn")).toObject());
            if (durationOnRes.is_err())
                return Err(durationOnRes.unwrap_err());
            preset.flashing.durationOn = *durationOnRes;

            auto durationOffRes = jsonToDuration(flashingJson.value(QLatin1String("durationOff")).toObject());
            if (durationOffRes.is_err())
                return Err(durationOffRes.unwrap_err());
            preset.flashing.durationOff = *durationOffRes;

            preset.flashing.power = flashingJson.value(QStringLiteral("power")).toInt();
        }

        return Ok(preset);
    }

    QString ExposureBaseViewModel::presetDetailsToStr(const PresetExposure &preset)
    {
        if (preset.mode == ExposureMode::Continuous)
        {
            QString secondsStr = QString::number(preset.continuous.duration.miliseconds / 1000.0, 'f', 1);
            return QStringLiteral("Exposure duration: %1min %2s\nExposure power: %3%")
                .arg(preset.continuous.duration.minutes)
                .arg(secondsStr)
                .arg(preset.continuous.power);
        }
        else
        {
            QString secondsOnStr  = QString::number(preset.flashing.durationOn.miliseconds / 1000.0, 'f', 1);
            QString secondsOffStr = QString::number(preset.flashing.durationOff.miliseconds / 1000.0, 'f', 1);
            return QStringLiteral("Number of cycles: %1\nDuration Ton: %2min %3s\nDuration Toff: %4min %5s\nExposure power: %6%")
                .arg(preset.flashing.numberOfCycles)
                .arg(preset.flashing.durationOn.minutes)
                .arg(secondsOnStr)
                .arg(preset.flashing.durationOff.minutes)
                .arg(secondsOffStr)
                .arg(preset.flashing.power);
        }
    }

    ExposureMode ExposureBaseViewModel::stringToMode(const QString &modeString)
    {
        if (modeString == QStringLiteral("Flashing"))
            return ExposureMode::Flashing;

        // Default to Continuous if the string is unrecognized
        return ExposureMode::Continuous;
    }

    Result<Duration, QString> ExposureBaseViewModel::jsonToDuration(const QJsonObject &json)
    {
        Duration duration;

        if (json.contains(QStringLiteral("minutes")))
            duration.minutes = json.value(QStringLiteral("minutes")).toInt();
        else
            return Err(QStringLiteral("The duration JSON object is missing the 'minutes' field."));

        if (json.contains(QStringLiteral("miliseconds")))
            duration.miliseconds = json.value(QStringLiteral("miliseconds")).toInt();
        else
            return Err(QStringLiteral("The duration JSON object is missing the 'miliseconds' field."));

        return Ok(duration);
    }

} // namespace Kub3::UI::ViewModels::Exposure