#include <ViewModels/Alignment/RenameParametersViewModel.h>

#include <ViewModels/Alignment/AlignmentParametersPersistence.h>

namespace Kub3::UI::ViewModels::Alignment
{

    RenameParametersViewModel::RenameParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    RenameParametersViewModel::~RenameParametersViewModel()
    {
    }

    void RenameParametersViewModel::ps_renameParameters(const QString &parameter)
    {
        m_currentNameParameter = parameter;
    }

    void RenameParametersViewModel::ui_userRequestRenameParameters(const QString &name)
    {
        m_newNameParameter = name;
        auto result        = replaceParameters();
        if (result.is_ok())
        {
            emit s_parameterRenamed();
        }
        else
        {
            emit s_errorRenamingParameter(result.unwrap_err());
        }
    }

    Result<Unit, const char *> RenameParametersViewModel::replaceParameters()
    {
        if (m_newNameParameter.trimmed().isEmpty())
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
        const bool parameterExists = Persistence::parameterExistsInFile(parametersArray, m_currentNameParameter);
        const bool newNameExists   = Persistence::parameterExistsInFile(parametersArray, m_newNameParameter);

        if (!parameterExists)
        {
            return Err("The parameter to rename does not exist.");
        }
        if (newNameExists)
        {
            return Err("A parameter with the new name already exists.");
        }
        if (!Persistence::changeParameterNameInFile(parametersArray, m_currentNameParameter, m_newNameParameter))
        {
            return Err("Failed to change parameter name.");
        }

        return Persistence::saveParametersToFile(path, parametersArray);
    }

    Result<QJsonObject, const char *> RenameParametersViewModel::getParameterByName(const QString &parameterName)
    {
        auto loadRes = Persistence::loadParametersFromFile(Persistence::storagePath());
        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        return Persistence::getParameterByName(loadRes.unwrap(), parameterName);
    }

    QString RenameParametersViewModel::getCurrentNameParameter() const
    {
        return m_currentNameParameter;
    }

} // namespace Kub3::UI::ViewModels::Alignment