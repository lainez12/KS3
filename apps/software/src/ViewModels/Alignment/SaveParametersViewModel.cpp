#include <ViewModels/Alignment/SaveParametersViewModel.h>

#include <ViewModels/Alignment/AlignmentParametersPersistence.h>

namespace Kub3::UI::ViewModels::Alignment
{

    SaveParametersViewModel::SaveParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    SaveParametersViewModel::~SaveParametersViewModel()
    {
    }

    Result<Unit, QString> SaveParametersViewModel::saveParameters(const QJsonObject &parameter, bool replaceExisting)
    {
        if (!parameter.contains(QStringLiteral("name")))
            return Err(QStringLiteral("The parameter JSON object is missing the 'name' field."));

        const QString parameterName = parameter.value(QStringLiteral("name")).toString().trimmed();
        if (parameterName.isEmpty())
            return Err(QStringLiteral("The parameter name cannot be empty."));

        const QString path = Persistence::storagePath();

        auto dirRes = Persistence::ensureParentDirectory(path);
        if (dirRes.is_err())
            return Err(dirRes.unwrap_err());

        auto loadRes = Persistence::loadParametersFromFile(path);
        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        QJsonArray parametersArray = loadRes.unwrap();
        const bool parameterExists  = Persistence::parameterExistsInFile(parametersArray, parameterName);

        if (parameterExists && !replaceExisting)
            return Err(QStringLiteral("A parameter set with the same name already exists. Please choose a different name or enable replacement."));

        if (!parameterExists)
            parametersArray.append(parameter);
        else
            Persistence::replaceExistingParameter(parametersArray, parameter);

        return Persistence::saveParametersToFile(path, parametersArray);
    }

    Result<QJsonArray, QString> SaveParametersViewModel::getAllParameters()
    {
        return Persistence::loadParametersFromFile(Persistence::storagePath());
    }

    Result<QJsonObject, QString> SaveParametersViewModel::getParameterByName(const QString &parameterName)
    {
        auto loadRes = Persistence::loadParametersFromFile(Persistence::storagePath());
        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        return Persistence::getParameterByName(loadRes.unwrap(), parameterName);
    }

} // namespace Kub3::UI::ViewModels::Alignment