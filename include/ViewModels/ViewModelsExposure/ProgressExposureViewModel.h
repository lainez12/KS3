#ifndef PROGRESSEXPOSUREVIEWMODEL_H
#define PROGRESSEXPOSUREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsExposure {
    class ProgressExposureViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit ProgressExposureViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ProgressExposureViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels

#endif