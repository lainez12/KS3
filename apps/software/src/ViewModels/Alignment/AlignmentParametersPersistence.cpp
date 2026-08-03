#include <ViewModels/Alignment/AlignmentParametersPersistence.h>

#include <Common/Result.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>

namespace Kub3::UI::ViewModels::Alignment::Persistence
{
    constexpr auto kRootKey          = "parameters";
    constexpr auto kVersionKey       = "version";
    constexpr auto kNameKey          = "name";
    constexpr auto kCameraLeftKey    = "cameraLeft";
    constexpr auto kCameraRightKey   = "cameraRight";
    constexpr auto kPositionKey      = "position";
    constexpr auto kVisualisationKey = "visualisation";
    constexpr auto kXKey             = "x";
    constexpr auto kYKey             = "y";
    constexpr auto kGainKey          = "gain";
    constexpr auto kExposureKey      = "exposure";
    constexpr auto kZoomKey          = "zoom";
    constexpr auto kFocusKey         = "focus";
    constexpr auto kLightKey         = "light";

    using AlignmentParameter = Kub3::UI::ViewModels::Alignment::Persistence::AlignmentParameter;

    QJsonObject cameraPositionToJson(const AlignmentParameter::CameraPosition &position)
    {
        QJsonObject json;
        json.insert(QLatin1String(kXKey), position.x);
        json.insert(QLatin1String(kYKey), position.y);
        return json;
    }

    Result<AlignmentParameter::CameraPosition, QString> jsonToCameraPosition(const QJsonObject &json)
    {
        AlignmentParameter::CameraPosition position;

        if (!json.contains(QLatin1String(kXKey)))
            return Err(QStringLiteral("The camera position JSON object is missing the 'x' field."));
        if (!json.contains(QLatin1String(kYKey)))
            return Err(QStringLiteral("The camera position JSON object is missing the 'y' field."));

        position.x = json.value(QLatin1String(kXKey)).toInt();
        position.y = json.value(QLatin1String(kYKey)).toInt();

        return Ok(position);
    }

    void insertOptionalInt(QJsonObject &json, const char *key, const std::optional<int> &value)
    {
        if (value.has_value())
            json.insert(QLatin1String(key), *value);
    }

    QJsonObject cameraVisualisationToJson(const AlignmentParameter::CameraVisualisation &visualisation)
    {
        QJsonObject json;
        insertOptionalInt(json, kGainKey, visualisation.gain);
        insertOptionalInt(json, kExposureKey, visualisation.exposure);
        insertOptionalInt(json, kZoomKey, visualisation.zoom);
        insertOptionalInt(json, kFocusKey, visualisation.focus);
        insertOptionalInt(json, kLightKey, visualisation.light);
        return json;
    }

    std::optional<int> optionalIntFromJson(const QJsonObject &json, const char *key)
    {
        if (!json.contains(QLatin1String(key)))
            return std::nullopt;

        return json.value(QLatin1String(key)).toInt();
    }

    AlignmentParameter::CameraVisualisation jsonToCameraVisualisation(const QJsonObject &json)
    {
        AlignmentParameter::CameraVisualisation visualisation;
        visualisation.gain     = optionalIntFromJson(json, kGainKey);
        visualisation.exposure = optionalIntFromJson(json, kExposureKey);
        visualisation.zoom     = optionalIntFromJson(json, kZoomKey);
        visualisation.focus    = optionalIntFromJson(json, kFocusKey);
        visualisation.light    = optionalIntFromJson(json, kLightKey);
        return visualisation;
    }

    QJsonObject cameraToJson(const AlignmentParameter::Camera &camera)
    {
        QJsonObject json;
        json.insert(QLatin1String(kPositionKey), cameraPositionToJson(camera.position));
        json.insert(QLatin1String(kVisualisationKey), cameraVisualisationToJson(camera.visualisation));
        return json;
    }

    Result<AlignmentParameter::Camera, QString> jsonToCamera(const QJsonObject &json)
    {
        AlignmentParameter::Camera camera;

        if (!json.contains(QLatin1String(kPositionKey)))
            return Err(QStringLiteral("The camera JSON object is missing the 'position' field."));

        auto positionRes = jsonToCameraPosition(json.value(QLatin1String(kPositionKey)).toObject());
        if (positionRes.is_err())
            return Err(positionRes.unwrap_err());

        camera.position = *positionRes;

        if (json.contains(QLatin1String(kVisualisationKey)))
            camera.visualisation = jsonToCameraVisualisation(json.value(QLatin1String(kVisualisationKey)).toObject());

        return Ok(camera);
    }
}

namespace Kub3::UI::ViewModels::Alignment::Persistence
{
    QString storagePath()
    {
        return QStringLiteral(KUB3_SAVES_PARAMETERS_ALIGNMENT_PATH);
    }

    Result<Unit, QString> ensureParentDirectory(const QString &path)
    {
        const QFileInfo fileInfo(path);
        const QDir directory = fileInfo.dir();

        if (directory.exists())
            return Ok<Unit>({});
        if (QDir().mkpath(directory.absolutePath()))
            return Ok<Unit>({});

        return Err(QStringLiteral("The destination directory could not be created: %1").arg(directory.absolutePath()));
    }

    Result<QJsonArray, QString> loadParametersFromFile(const QString &path)
    {
        QFile inputFile(path);

        if (!inputFile.exists())
            return Ok(QJsonArray{});

        if (!inputFile.open(QIODevice::ReadOnly))
            return Err(QStringLiteral("The parameters file could not be opened for reading: %1").arg(inputFile.errorString()));

        const QByteArray rawData = inputFile.readAll();
        inputFile.close();

        if (rawData.isEmpty())
            return Ok(QJsonArray{});

        QJsonParseError parseError{};
        const QJsonDocument document = QJsonDocument::fromJson(rawData, &parseError);

        if (parseError.error != QJsonParseError::NoError)
            return Err(QStringLiteral("The parameters file has an invalid JSON format: %1").arg(parseError.errorString()));

        if (document.isArray())
            return Ok(document.array());

        if (document.isObject())
        {
            const QJsonObject rootObject = document.object();
            return Ok(rootObject.value(QLatin1String(kRootKey)).toArray());
        }

        return Ok(QJsonArray{});
    }

    Result<Unit, QString> saveParametersToFile(const QString &path, const QJsonArray &parametersArray)
    {
        QJsonObject rootObject;
        rootObject.insert(QLatin1String(kVersionKey), 1);
        rootObject.insert(QLatin1String(kRootKey), parametersArray);

        QSaveFile outputFile(path);
        if (!outputFile.open(QIODevice::WriteOnly))
            return Err(QStringLiteral("The parameters file could not be opened for writing: %1").arg(outputFile.errorString()));

        outputFile.write(QJsonDocument(rootObject).toJson(QJsonDocument::Indented));
        if (!outputFile.commit())
            return Err(QStringLiteral("The parameters file could not be saved: %1").arg(outputFile.errorString()));

        return Ok<Unit>({});
    }

    QJsonObject parameterToJson(const AlignmentParameter &parameter)
    {
        QJsonObject json;
        json.insert(QLatin1String(kNameKey), parameter.name);
        json.insert(QLatin1String(kCameraLeftKey), cameraToJson(parameter.cameraLeft));
        json.insert(QLatin1String(kCameraRightKey), cameraToJson(parameter.cameraRight));
        return json;
    }

    Result<AlignmentParameter, QString> jsonToParameter(const QJsonObject &json)
    {
        AlignmentParameter parameter;

        if (!json.contains(QLatin1String(kNameKey)))
            return Err(QStringLiteral("The parameter JSON object is missing the 'name' field."));
        if (!json.contains(QLatin1String(kCameraLeftKey)))
            return Err(QStringLiteral("The parameter JSON object is missing the 'cameraLeft' field."));
        if (!json.contains(QLatin1String(kCameraRightKey)))
            return Err(QStringLiteral("The parameter JSON object is missing the 'cameraRight' field."));

        parameter.name = json.value(QLatin1String(kNameKey)).toString().trimmed();

        auto cameraLeftRes = jsonToCamera(json.value(QLatin1String(kCameraLeftKey)).toObject());
        if (cameraLeftRes.is_err())
            return Err(cameraLeftRes.unwrap_err());
        parameter.cameraLeft = *cameraLeftRes;

        auto cameraRightRes = jsonToCamera(json.value(QLatin1String(kCameraRightKey)).toObject());
        if (cameraRightRes.is_err())
            return Err(cameraRightRes.unwrap_err());
        parameter.cameraRight = *cameraRightRes;

        if (parameter.name.isEmpty())
            return Err(QStringLiteral("The parameter name cannot be empty."));

        return Ok(parameter);
    }

    bool parameterExistsInFile(const QJsonArray &parametersArray, const QString &parameterName)
    {
        for (const QJsonValue &value : parametersArray)
        {
            const QJsonObject parameterObject = value.toObject();
            if (parameterObject.value(QLatin1String(kNameKey)).toString() == parameterName)
                return true;
        }
        return false;
    }

    void replaceExistingParameter(QJsonArray &parametersArray, const QJsonObject &parameter)
    {
        const QString parameterName = parameter.value(QLatin1String(kNameKey)).toString();

        for (int i = 0; i < parametersArray.size(); ++i)
        {
            const QJsonObject parameterObject = parametersArray[i].toObject();
            if (parameterObject.value(QLatin1String(kNameKey)).toString() == parameterName)
            {
                parametersArray[i] = parameter;
                break;
            }
        }
    }

    Result<QJsonObject, QString> getParameterByName(const QJsonArray &parametersArray, const QString &parameterName)
    {
        for (const QJsonValue &value : parametersArray)
        {
            const QJsonObject parameterObject = value.toObject();

            if (parameterObject.value(QLatin1String(kNameKey)).toString() == parameterName)
                return Ok(parameterObject);
        }

        return Err(QStringLiteral("The parameter set with name '%1' was not found.").arg(parameterName));
    }

    bool deleteByName(QJsonArray &parametersArray, const QString &parameterName)
    {
        for (int i = 0; i < parametersArray.size(); ++i)
        {
            const QJsonObject parameterObject = parametersArray[i].toObject();
            if (parameterObject.value(QLatin1String(kNameKey)).toString() == parameterName)
            {
                parametersArray.removeAt(i);
                return true;
            }
        }
        return false;
    }
}