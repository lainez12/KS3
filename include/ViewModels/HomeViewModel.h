#pragma once

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels {
    class HomeViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~HomeViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    signals:

    private slots:

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };
}