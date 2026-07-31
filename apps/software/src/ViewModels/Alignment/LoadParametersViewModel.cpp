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

    Result<QJsonArray, QString> LoadParametersViewModel::getAllParameters()
    {
        return Persistence::loadParametersFromFile(Persistence::storagePath());
    }

    Result<QJsonObject, QString> LoadParametersViewModel::getParameterByName(const QString &parameterName)
    {
        auto loadRes = Persistence::loadParametersFromFile(Persistence::storagePath());
        if (loadRes.is_err())
            return Err(loadRes.unwrap_err());

        return Persistence::getParameterByName(loadRes.unwrap(), parameterName);
    }

} // namespace Kub3::UI::ViewModels::Alignment