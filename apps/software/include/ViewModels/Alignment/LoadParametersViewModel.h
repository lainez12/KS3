#pragma once

#include <Common/Result.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <ViewModels/Alignment/AlignmentParametersPersistence.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class LoadParametersViewModel final : public BaseViewModel
    {
        Q_OBJECT

        using alignment_parameter_t = Persistence::alignment_parameter_t;

    public:
        explicit LoadParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~LoadParametersViewModel() override;

    signals:
        void s_loadParameterSelected(const alignment_parameter_t &parameter);
        void s_renameParameterSelected(const QString &parameterName);

    public:
        Result<QJsonArray, const char *> getAllParameters();
        Result<QJsonObject, const char *> getParameterByName(const QString &parameterName);
        Result<QList<QString>, const char *> getAllNamesSavedParameters();
        Result<Unit, const char *> removeParameterByName(const QString &parameterName);
        bool uiRequestedLoadParameter(const QString &parameterName);
        void uiRequestedRenameParameter(const QString &parameterName);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Alignment