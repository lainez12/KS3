#ifndef UPDATESOFTWAREVIEWMODEL_H
#define UPDATESOFTWAREVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    class UpdateSoftwareViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit UpdateSoftwareViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~UpdateSoftwareViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewsModelSettings

#endif