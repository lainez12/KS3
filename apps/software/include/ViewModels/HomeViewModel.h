#pragma once

#include <Common/Enums.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <MFSM/MasterFSM.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{
    class HomeViewModel : public BaseViewModel
    {
        Q_OBJECT
    public:
        explicit HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);

        void loadConnections() override; // Hook to push state to the View

    signals:
        // Signals to the view
        void s_setDrawerActionsLock(bool lock);
        void s_setExposureModeLock(bool lock);
        void s_setHomingLock(bool lock);
        void s_setInitializationLock(bool lock);

        // Signals to the FSM
        void s_cancelOperation();
        void s_initializationRequest();
        void s_cmdRunDrawerOperation(DrawerTarget tgt, bool eject);

    public slots:
        // From the view
        void uiRequestCancel();
        void uiRequestInitialization();
        void uiRequestDrawerOperation(DrawerTarget tgt, bool isEject);

        // From the FSM
        void ps_onSystemStateChanged(MFSM::SystemStateKind state);
        void ps_onOperationalSubstateKindChanged(MFSM::OperationalStateKind state);
        void ps_initializationSuccess();
        void ps_errorOccurred(const MFSM::ErrorPayload &payload);
        void ps_operationEnded();

    private:
        void evaluateLocks();
        void evaluatePopup(MFSM::SystemStateKind prevState);

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        MFSM::SystemPosture m_posture;
        MFSM::SystemStateKind m_currentSysState        = MFSM::SystemStateKind::Booting;
        MFSM::OperationalStateKind m_currentOpSubstate = MFSM::OperationalStateKind::Idle;
    };
}