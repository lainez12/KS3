#pragma once

#include <Common/Result.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <ViewModels/Alignment/AlignmentParametersPersistence.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class RenameParametersViewModel final : public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit RenameParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~RenameParametersViewModel() override;

    signals:
        void s_parameterRenamed();
        void s_errorRenamingParameter(const QString &errorMessage);

    public slots:
        void ps_renameParameters(const QString &currentName);

    public:
        void ui_userRequestRenameParameters(const QString &name);
        Result<QJsonObject, const char *> getParameterByName(const QString &parameterName);

    private:
        Result<Unit, const char *> replaceParameters();

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        QString m_currentNameParameter;
        QString m_newNameParameter;
    };

} // namespace Kub3::UI::ViewModels::Alignment