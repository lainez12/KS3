#pragma once

#include <Common/Enums.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <MFSM/MasterFSM.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{
    class ExposureModeViewModel : public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit ExposureModeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);

    signals:
        // Signals to the view
        void s_setAlignmentViewLock(bool lock);
        void s_setFloodExposureLock(bool lock);

    public slots:
        // From the FSM
        void ps_onSystemStateChanged(MFSM::SystemStateKind stateKind);
        void ps_onPostureChanged(const MFSM::SystemPosture &posture);

    private:
        void updateView();

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        MFSM::SystemPosture m_posture;
        MFSM::SystemStateKind m_currentSysState = MFSM::SystemStateKind::Booting;
    };
}
