#ifndef SAVEPARAMETERSVIEWMODEL_H
#define SAVEPARAMETERSVIEWMODEL_H

#include <Common/Result.h>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>
#include <ViewModels/Alignment/AlignmentParametersPersistence.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class SaveParametersViewModel final : public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit SaveParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveParametersViewModel() override;

    public slots:
        void ps_saveParameters(const Persistence::AlignmentParameter &parameter);

    public:
        Result<Unit, QString> ui_userRequestSaveParameters(const QString &name, bool replaceExisting = true);
        Result<QList<QString>, QString> getAllNamesSavedParameters();
        Result<QJsonArray, QString> getAllParameters();
        Result<QJsonObject, QString> getParameterByName(const QString &parameterName);

    private:
        Result<Unit, QString> saveParameters(const Persistence::AlignmentParameter &parameter, bool replaceExisting = true);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Persistence::AlignmentParameter m_currentParameter;
    };

} // namespace Kub3::UI::ViewModels::Alignment

#endif