#ifndef ALIGNMENTPARAMETERSPERSISTENCE_H
#define ALIGNMENTPARAMETERSPERSISTENCE_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <Common/Result.h>

namespace Kub3::UI::ViewModels::Alignment::Persistence
{
    QString storagePath();

    Result<Unit, QString> ensureParentDirectory(const QString &path);
    Result<QJsonArray, QString> loadParametersFromFile(const QString &path);
    Result<Unit, QString> saveParametersToFile(const QString &path, const QJsonArray &parametersArray);

    bool parameterExistsInFile(const QJsonArray &parametersArray, const QString &parameterName);
    void replaceExistingParameter(QJsonArray &parametersArray, const QJsonObject &parameter);
    Result<QJsonObject, QString> getParameterByName(const QJsonArray &parametersArray, const QString &parameterName);
    bool deleteByName(QJsonArray &parametersArray, const QString &parameterName);
}

#endif