#pragma once

#include <Common/Enums.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <MFSM/MasterFSM.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels
{
    class HomeViewModel final : public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~HomeViewModel() override;

    signals:
        // Send to logic layer
        void s_cancelOperation();
        void s_initializationRequest();
        void s_cmdRunDrawerOperation(DrawerTarget tgt, bool eject);
        // Send to UI
        void s_setDrawerActionsLock(bool lock);
        void s_setExposureModeLock(bool lock);
        void s_setHomingLock(bool lock);
        void s_setInitializationLock(bool lock);

    public slots:
        void uiRequestCancel();
        void uiRequestInitialization();
        void uiRequestDrawerOperation(DrawerTarget tgt, bool eject);

    public slots:
        void ps_initializationSuccess();
        void ps_errorOccurred(const Kub3::MFSM::ErrorPayload &payload);
        void ps_operationEnded();

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };
}