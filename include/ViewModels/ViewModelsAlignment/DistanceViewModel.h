#ifndef DISTANCEVIEWMODEL_H
#define DISTANCEVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsAlignment {
    class DistanceViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit DistanceViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~DistanceViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewModelsAlignment

#endif