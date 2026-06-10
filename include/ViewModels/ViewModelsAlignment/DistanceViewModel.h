#ifndef DISTANCEVIEWMODEL_H
#define DISTANCEVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsAlignment
{
    class DistanceViewModel final : public QObject, public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit DistanceViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~DistanceViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewModelsAlignment

#endif