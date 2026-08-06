#pragma once

#include <optional>

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <Common/Result.h>

namespace Kub3::UI::ViewModels::Alignment::Persistence
{
    typedef struct camera_position_s {
        double x = 0;
        double y = 0;
    } camera_position_t;

    typedef struct camera_visualisation_s {
        std::optional<double> gain;
        std::optional<double> exposure;
        std::optional<double> zoom;
        std::optional<double> focus;
        std::optional<double> light;
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

    Result<Unit, const char *> ensureParentDirectory(const QString &path);
    Result<QJsonArray, const char *> loadParametersFromFile(const QString &path);
    Result<Unit, const char *> saveParametersToFile(const QString &path, const QJsonArray &parametersArray);

    QJsonObject parameterToJson(const alignment_parameter_t &parameter);
    Result<alignment_parameter_t, const char *> jsonToParameter(const QJsonObject &json);

    bool parameterExistsInFile(const QJsonArray &parametersArray, const QString &parameterName);
    void replaceExistingParameter(QJsonArray &parametersArray, const QJsonObject &parameter);
    bool changeParameterNameInFile(QJsonArray &parametersArray, const QString &oldName, const QString &newName);
    Result<QJsonObject, const char *> getParameterByName(const QJsonArray &parametersArray, const QString &parameterName);
    bool deleteByName(QJsonArray &parametersArray, const QString &parameterName);
}