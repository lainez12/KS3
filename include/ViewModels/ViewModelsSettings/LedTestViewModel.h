#ifndef LEDTESTVIEWMODEL_H
#define LEDTESTVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    class LedTestViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit LedTestViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~LedTestViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewsModelSettings

#endif