#ifndef OPERATINGTIMESVIEWMODEL_H
#define OPERATINGTIMESVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsSettings {
    class OperatingTimesViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit OperatingTimesViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~OperatingTimesViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewModelsSettings

#endif