#include <ViewModels/Alignment/LoadParametersViewModel.h>

#include <ViewModels/Alignment/AlignmentParametersPersistence.h>

namespace Kub3::UI::ViewModels::Alignment
{
    LoadParametersViewModel::LoadParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    LoadParametersViewModel::~LoadParametersViewModel()
    {
    }

    Result<QJsonArray, const char *> LoadParametersViewModel::getAllParameters()
    {
        return Persistence::loadParametersFromFile(Persistence::storagePath());
    }

    Result<QJsonObject, const char *> LoadParametersViewModel::getParameterByName(const QString &parameterName)
    {
        auto loadRes = Persistence::loadParametersFromFile(Persistence::storagePath());
        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        return Persistence::getParameterByName(loadRes.unwrap(), parameterName);
    }

    Result<QList<QString>, const char *> LoadParametersViewModel::getAllNamesSavedParameters()
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

} // namespace Kub3::UI::ViewModels::Alignment