#pragma once

#include <optional>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <Common/Result.h>

namespace Kub3::UI::ViewModels::Alignment::Persistence
{
    struct AlignmentParameter
    {
        struct CameraPosition
        {
            int x = 0;
            int y = 0;
        };

        struct CameraVisualisation
        {
            std::optional<int> gain;
            std::optional<int> exposure;
            std::optional<int> zoom;
            std::optional<int> focus;
            std::optional<int> light;
        };

        struct Camera
        {
            CameraPosition position;
            CameraVisualisation visualisation;
        };

        QString name;
        Camera cameraLeft;
        Camera cameraRight;
    };

    QString storagePath();

    Result<Unit, QString> ensureParentDirectory(const QString &path);
    Result<QJsonArray, QString> loadParametersFromFile(const QString &path);
    Result<Unit, QString> saveParametersToFile(const QString &path, const QJsonArray &parametersArray);

    QJsonObject parameterToJson(const AlignmentParameter &parameter);
    Result<AlignmentParameter, QString> jsonToParameter(const QJsonObject &json);

    bool parameterExistsInFile(const QJsonArray &parametersArray, const QString &parameterName);
    void replaceExistingParameter(QJsonArray &parametersArray, const QJsonObject &parameter);
    Result<QJsonObject, QString> getParameterByName(const QJsonArray &parametersArray, const QString &parameterName);
    bool deleteByName(QJsonArray &parametersArray, const QString &parameterName);
}