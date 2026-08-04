#pragma once

#include <optional>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <Common/Result.h>

namespace Kub3::UI::ViewModels::Alignment::Persistence
{
    typedef struct camera_position_s {
        int x = 0;
        int y = 0;
    } camera_position_t;

    typedef struct camera_visualisation_s {
        std::optional<int> gain;
        std::optional<int> exposure;
        std::optional<int> zoom;
        std::optional<int> focus;
        std::optional<int> light;
    } camera_visualisation_t;

    typedef struct camera {
        camera_position_t position;
        camera_visualisation_t visualisation;
    } camera_t;

    typedef struct alignment_parameter_s {
        QString name;
        camera_t cameraLeft;
        camera_t cameraRight;
    } alignment_parameter_t;

    QString storagePath();

    Result<Unit, QString> ensureParentDirectory(const QString &path);
    Result<QJsonArray, QString> loadParametersFromFile(const QString &path);
    Result<Unit, QString> saveParametersToFile(const QString &path, const QJsonArray &parametersArray);

    QJsonObject parameterToJson(const alignment_parameter_t &parameter);
    Result<alignment_parameter_t, QString> jsonToParameter(const QJsonObject &json);

    bool parameterExistsInFile(const QJsonArray &parametersArray, const QString &parameterName);
    void replaceExistingParameter(QJsonArray &parametersArray, const QJsonObject &parameter);
    Result<QJsonObject, QString> getParameterByName(const QJsonArray &parametersArray, const QString &parameterName);
    bool deleteByName(QJsonArray &parametersArray, const QString &parameterName);
}