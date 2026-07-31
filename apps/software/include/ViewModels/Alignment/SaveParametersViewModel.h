#ifndef SAVEPARAMETERSVIEWMODEL_H
#define SAVEPARAMETERSVIEWMODEL_H

#include <Common/Result.h>
#include <QJsonArray>
#include <QJsonObject>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{
    class SaveParametersViewModel final : public BaseViewModel
    {

    public:
        explicit SaveParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveParametersViewModel() override;

        Result<Unit, QString> saveParameters(const QJsonObject &parameter, bool replaceExisting = true);
        Result<QJsonArray, QString> getAllParameters();
        Result<QJsonObject, QString> getParameterByName(const QString &parameterName);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Alignment

#endif