#include <ViewModels/Exposure/SaveExposureSettingsViewModel.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTextStream>

namespace
{
    constexpr auto kRootKey       = "presets";
    constexpr auto kVersionKey    = "version";
    constexpr auto kNameKey       = "name";
    constexpr auto kModeKey       = "mode";
    constexpr auto kContinuousKey = "continuous";
    constexpr auto kFlashingKey   = "flashing";
}

namespace Kub3::UI::ViewModels::Exposure
{
    SaveExposureSettingsViewModel::SaveExposureSettingsViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        ExposureBaseViewModel(repo, parent)
    {
    }

    SaveExposureSettingsViewModel::~SaveExposureSettingsViewModel()
    {
    }

    bool SaveExposureSettingsViewModel::savePreset(const PresetExposure &preset, QString *errorMessage)
    {
        if (!validatePreset(preset, errorMessage))
            return false;

        const QString path = storagePath();
        if (!ensureParentDirectory(path, errorMessage))
            return false;

        QJsonArray presetsArray;

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
                    presetsArray = document.array();
                }
                else if (document.isObject())
                {
                    const QJsonObject rootObject = document.object();
                    presetsArray                 = rootObject.value(QLatin1String(kRootKey)).toArray();
                }
            }
        }

        bool replacedExistingPreset = false;
        for (int index = 0; index < presetsArray.size(); ++index)
        {
            const QJsonObject currentPreset = presetsArray.at(index).toObject();
            if (currentPreset.value(QLatin1String(kNameKey)).toString() == preset.name)
            {
                presetsArray[index]    = presetToJson(preset);
                replacedExistingPreset = true;
                break;
            }
        }

        if (!replacedExistingPreset)
            presetsArray.append(presetToJson(preset));

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

    QString SaveExposureSettingsViewModel::storagePath()
    {
        return QStringLiteral(KUB3_SAVES_EXPOSURE_SETTINGS_PATH);
    }

    QString SaveExposureSettingsViewModel::modeToString(ExposureMode mode)
    {
        switch (mode)
        {
        case ExposureMode::Continuous:
            return QStringLiteral("continuous");
        case ExposureMode::Flashing:
            return QStringLiteral("flashing");
        }

        return QStringLiteral("continuous");
    }

    QJsonObject SaveExposureSettingsViewModel::durationToJson(const Duration &duration)
    {
        QJsonObject json;
        json.insert(QStringLiteral("minutes"), duration.minutes);
        json.insert(QStringLiteral("seconds"), duration.seconds);
        return json;
    }

    QJsonObject SaveExposureSettingsViewModel::presetToJson(const PresetExposure &preset)
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

    bool SaveExposureSettingsViewModel::validatePreset(const PresetExposure &preset, QString *errorMessage)
    {
        if (preset.name.trimmed().isEmpty())
        {
            if (errorMessage)
                *errorMessage = QStringLiteral("The preset name cannot be empty.");
            return false;
        }

        const auto validateDuration = [errorMessage](const Duration &duration, const QString &label) {
            if (duration.minutes < 0 || duration.seconds < 0 || duration.seconds >= 60)
            {
                if (errorMessage)
                    *errorMessage = QStringLiteral("The duration %1 must have minutes >= 0 and seconds between 0 and 59.").arg(label);
                return false;
            }

            return true;
        };

        if (preset.mode == ExposureMode::Continuous)
        {
            if (!validateDuration(preset.continuous.duration, QStringLiteral("continuous")))
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

    bool SaveExposureSettingsViewModel::ensureParentDirectory(const QString &path, QString *errorMessage)
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

} // namespace Kub3::UI::ViewModels::Exposure