#ifndef LOADPARAMETERSVIEWMODEL_H
#define LOADPARAMETERSVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsAlignment {
    class LoadParametersViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit LoadParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~LoadParametersViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewModelsAlignment

#endif