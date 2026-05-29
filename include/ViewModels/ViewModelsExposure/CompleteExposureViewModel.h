#ifndef COMPLETEEXPOSUREVIEWMODEL_H
#define COMPLETEEXPOSUREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure {
    class CompleteExposureViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit CompleteExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~CompleteExposureViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewModelsExposure

#endif