#pragma once

#include <Common/Enums.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <MFSM/states.macro.h>
#include <MFSM/states.operational.h>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::Views
{
    class ExposureMenuView;
}

namespace Kub3::UI::ViewModels
{

    enum class Operation : uint32_t
    {
        NONE          = 0u,
        STOWING_MASK  = 1u << 0,
        STOWING_WAFER = 1u << 1,
        AUTOLEVELING  = 1u << 2
    };
    impl_bin_operators_for_enum(Operation);

    class ExposureMenuViewModel final : public BaseViewModel
    {
        Q_OBJECT

    public:
        explicit ExposureMenuViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);

        void loadConnections() override; // Hook to push state to the View

    public:
        enum class StepStatus
        {
            NotStarted,
            Running,
            Success,
            Error
        };

    signals:
        // --- To the View ---
        // UI actions Locks
        void s_setAllLocks(bool locked);
        void s_setMaskLoadLock(bool locked);
        void s_setMaskCancelLock(bool locked);
        void s_setWaferLoadLock(bool locked);
        void s_setWaferCancelLock(bool locked);
        void s_setAutolevelLock(bool locked);
        void s_setAutolevelCancelLock(bool locked);
        void s_setSaveHorizontalityLock(bool locked);
        void s_setLoadHorizontalityLock(bool locked);
        void s_setContinueToExposureLock(bool locked);
        // UI actions statuses
        void s_setMaskStatus(StepStatus status);
        void s_setWaferStatus(StepStatus status);
        void s_setAutolevelStatus(StepStatus status);
        // UI log messages
        void s_appendMaskSequenceLog(const QString &htmlLogLine);
        void s_appendWaferSequenceLog(const QString &htmlLogLine);
        void s_appendAutolevelSequenceLog(const QString &htmlLogLine);
        void s_clearMaskSequenceLogs();
        void s_clearWaferSequenceLogs();
        void s_clearAutolevelSequenceLogs();

        // --- To the FSM ---
        void s_cmdOperateStowage(StowageTarget tgt);
        void s_cmdStartAutolevel();
        void s_cmdCancelOperation();

    public slots:
        // --- From the View ---
        void ui_requestLoadMaskToExposure();
        void ui_requestLoadWaferToExposure();
        void ui_requestAutoleveling();
        void ui_requestCancel();

        // --- From the FSM ---
        void ps_onSystemStateChanged(MFSM::SystemStateKind stateKind);
        void ps_onOperationalSubstateChanged(const MFSM::OperationalState &state);
        void ps_onPostureChanged(const MFSM::SystemPosture &posture);
        void ps_onProcessMessageBroadcast(const Kub3::Common::ProcessMessage &msg) override;

    private:
        void updateView();
        Operation currentOperation() const;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        MFSM::SystemStateKind m_currentSysState    = MFSM::SystemStateKind::Booting;
        MFSM::OperationalState m_currentOpSubstate = MFSM::StateIdle{};
        MFSM::SystemPosture m_machinePosture       = MFSM::SystemPosture{};
    };
}