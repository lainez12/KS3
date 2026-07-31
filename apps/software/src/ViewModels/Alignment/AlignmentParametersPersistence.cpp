#include <ViewModels/Alignment/AlignmentParametersPersistence.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>

namespace
{
    constexpr auto kRootKey    = "parameters";
    constexpr auto kVersionKey = "version";
    constexpr auto kNameKey    = "name";
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