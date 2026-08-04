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

    void SaveParametersViewModel::ps_saveParameters(const Persistence::alignment_parameter_t &parameter)
    {
        m_currentParameter = parameter;
    }

    void SaveParametersViewModel::ui_userRequestSaveParameters(const QString &name, bool replaceExisting)
    {
        m_currentParameter.name = name;
        auto result             = saveParameters(m_currentParameter, replaceExisting);
        if (result.is_ok())
        {
            emit s_parameterSaved();
        }
        else
        {
            emit s_errorSavingParameter(result.unwrap_err());
        }
    }

    Result<Unit, const char *> SaveParametersViewModel::saveParameters(const Persistence::alignment_parameter_t &parameter, bool replaceExisting)
    {
        if (parameter.name.trimmed().isEmpty())
            return Err("The parameter name cannot be empty.");

        static thread_local QByteArray errorStorage;

        const QString path = Persistence::storagePath();

        auto dirRes = Persistence::ensureParentDirectory(path);
        if (dirRes.is_err())
        {
            errorStorage = dirRes.unwrap_err();
            return Err(errorStorage.constData());
        }

        auto loadRes = Persistence::loadParametersFromFile(path);
        if (loadRes.is_err())
        {
            errorStorage = loadRes.unwrap_err();
            return Err(errorStorage.constData());
        }

        QJsonArray parametersArray = loadRes.unwrap();
        const bool parameterExists = Persistence::parameterExistsInFile(parametersArray, parameter.name);

        if (parameterExists && !replaceExisting)
            return Err("A parameter set with the same name already exists. Please choose a different name or enable replacement.");

        if (!parameterExists)
            parametersArray.append(Persistence::parameterToJson(parameter));
        else
            Persistence::replaceExistingParameter(parametersArray, Persistence::parameterToJson(parameter));

        return Persistence::saveParametersToFile(path, parametersArray);
    }

    Result<QList<QString>, const char *> SaveParametersViewModel::getAllNamesSavedParameters()
    {
        auto loadRes = Persistence::loadParametersFromFile(Persistence::storagePath());
        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        QList<QString> names;
        for (const QJsonValue &value : loadRes.unwrap())
        {
            if (!value.isObject())
                continue;

            auto parameterRes = Persistence::jsonToParameter(value.toObject());
            if (parameterRes.is_err())
                continue;

            names.append(parameterRes.unwrap().name);
        }

        return Ok(names);
    }

    Result<QJsonArray, const char *> SaveParametersViewModel::getAllParameters()
    {
        return Persistence::loadParametersFromFile(Persistence::storagePath());
    }

    Result<QJsonObject, const char *> SaveParametersViewModel::getParameterByName(const QString &parameterName)
    {
        auto loadRes = Persistence::loadParametersFromFile(Persistence::storagePath());
        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        return Persistence::getParameterByName(loadRes.unwrap(), parameterName);
    }

} // namespace Kub3::UI::ViewModels::Alignment