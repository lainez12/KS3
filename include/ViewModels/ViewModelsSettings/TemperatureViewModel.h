#ifndef TEMPERATUREVIEWMODEL_H
#define TEMPERATUREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    class TemperatureViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit TemperatureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~TemperatureViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewsModelSettings

#endif