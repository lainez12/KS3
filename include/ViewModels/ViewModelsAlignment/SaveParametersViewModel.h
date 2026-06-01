#ifndef SAVEPARAMETERSVIEWMODEL_H
#define SAVEPARAMETERSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsAlignment {
    class SaveParametersViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit SaveParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~SaveParametersViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewModelsAlignment

#endif