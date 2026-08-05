#pragma once

#include <Common/Result.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class LoadParametersViewModel final : public BaseViewModel
    {

    public:
        explicit LoadParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~LoadParametersViewModel() override;

        Result<QJsonArray, const char *> getAllParameters();
        Result<QJsonObject, const char *> getParameterByName(const QString &parameterName);
        Result<QList<QString>, const char *> getAllNamesSavedParameters();
        Result<Unit, const char *> removeParameterByName(const QString &parameterName);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Alignment