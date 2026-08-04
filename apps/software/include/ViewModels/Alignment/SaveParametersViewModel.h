#ifndef SAVEPARAMETERSVIEWMODEL_H
#define SAVEPARAMETERSVIEWMODEL_H

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
    class SaveParametersViewModel final : public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit SaveParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveParametersViewModel() override;

    signals:
        void s_parameterSaved();
        void s_errorSavingParameter(const QString &errorMessage);

    public slots:
        void ps_saveParameters(const Persistence::alignment_parameter_t &parameter);

    public:
        void ui_userRequestSaveParameters(const QString &name, bool replaceExisting = true);
        Result<QList<QString>, const char *> getAllNamesSavedParameters();
        Result<QJsonArray, const char *> getAllParameters();
        Result<QJsonObject, const char *> getParameterByName(const QString &parameterName);

    private:
        Result<Unit, const char *> saveParameters(const Persistence::alignment_parameter_t &parameter, bool replaceExisting = true);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Persistence::alignment_parameter_t m_currentParameter;
    };

} // namespace Kub3::UI::ViewModels::Alignment

#endif