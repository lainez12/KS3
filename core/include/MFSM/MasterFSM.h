#pragma once

#include <QObject>
#include <QRect>
#include <QString>
#include <QTimer>

#include <Common/ProcessMessage.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <MFSM/events.h>
#include <MFSM/posture.h>
#include <MFSM/states.macro.h>
#include <MFSM/states.operational.h>
#include <Services/Alignment/IAlignmentService.h>
#include <Services/Contact/IContactService.h>
#include <Services/Drawers/IDrawerService.h>
#include <Services/Exposure/IExposureService.h>
#include <Services/Homing/IHomingService.h>
#include <Services/Stowage/IStowageService.h>
#include <Services/Vision/IVisionService.h>

#define LOGIC_TIMER_PERIOD_MS 20u

namespace Kub3::MFSM
{

    struct ErrorPayload {
        ErrorKind kind;
        ErrorSeverity severity;
        QString message;
        ErrorAction allowedActions;
    };

    class MasterFSM final : public QObject
    {
        Q_OBJECT
    public:
        explicit MasterFSM(Shared<HAL::MS::IMachineStatusRepo> repo,
                           Shared<Services::IHomingService> homingService,
                           Shared<Services::IDrawerService> drawerService,
                           Shared<Services::IStowageService> stowageService,
                           Shared<Services::IAlignmentService> alignmentService,
                           Shared<Services::IVisionService> visionService,
                           Shared<Services::IContactService> contactService,
                           Shared<Services::IExposureService> exposureService,
                           QObject *parent = nullptr);
        ~MasterFSM(void) override = default;

        // Starts the internal 50Hz logic loop
        void start(void);

    signals:
        // --- Tier 2 (Logic) -> Tier 1 (UI) Outputs ---
        void s_systemStateKindChanged(SystemStateKind kind);
        void s_operationalSubstateKindChanged(OperationalStateKind kind);
        void s_operationalSubstateChanged(const OperationalState &state);
        void s_postureChanged(const MFSM::SystemPosture &posture); // To drive UI indicators
        void s_operationCanceled();
        // Logging/telemetry
        void s_processMessageBroadcast(const Common::ProcessMessage &msg);
        void s_clearProcessLogs(); // To wipe the QTextBrowser when a new sequence starts
        void s_initializationSuccess();
        void s_serviceOpSuccess();
        void s_serviceOpError(const QString &);

        // Error Management
        void s_warningOccurred(const QString &warningMessage); // E.g., Interlock rejections
        void s_errorOccurred(const ErrorPayload &payload);     // Hard faults / E-Stops

        // --- Tier 2 (Logic) -> Tier 3 (I/O threads) ---
        void s_requestHardwareRetry(const QString &hardwareId);
        void s_requestPowerOff(void);
        void s_requestCameraParamUpdate(const QString &camId, HAL::Vision::CameraParamKind kind, HAL::Vision::CameraParam value);

    public slots:
        // --- Tier 1 (UI) -> Tier 2 (Logic) Thread-Safe Commands ---

        // System lifecycle
        void ps_requestRetryBoot(void);
        void ps_requestAbortOperation(void);
        void ps_requestInitialization(void);
        void ps_requestResetError(void);
        void ps_requestEmergencyStop(void);
        void ps_systemPowerOff(void);

        // TODO: define correct parameter types
        // Hardware sequences
        void ps_requestOperateDrawer(DrawerTarget tgt, bool eject);
        void ps_requestStowage(StowageTarget tgt);
        void ps_requestAutolevel();
        void ps_requestUnstowage(StowageTarget tgt);
        void ps_requestExposure(const Services::ExposurePayload &payload);

        // Camera configuration
        void ps_requestExposureUpdate(const QString &camId, double val);
        void ps_requestGainUpdate(const QString &camId, double val);
        void ps_requestFrameRateUpdate(const QString &camId, double val);
        void ps_requestCenteredZoomUpdate(const QString &camId, double val);
        void ps_requestROIUpdate(const QString &camId, const QRect &roi);
        // Camera PAD movements
        // void ps_requestPADCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void ps_requestPADAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);

    private slots:
        // --- The Heartbeat (50Hz) ---
        void onLogicTick(void);

    private:
        // --- HFSM Core Dispatchers ---
        void dispatch(const SystemEvent &event);

        // Static events (do not change state index, e.g., joystick movements)
        [[nodiscard]] bool processStaticEvent(SystemState &currentState, const SystemEvent &event);

        // Transition Math (Pure Functions)
        [[nodiscard]] SystemState processMacroTransition(const SystemState &currentState, const SystemEvent &event);
        [[nodiscard]] OperationalState processOperationalTransition(const StateOperational &opState, const SystemEvent &event);

        // State Entry Triggers (Side effects, service routing)
        void onStateEntered(const SystemState &newState);
        void onOperationalSubstateEntered(const StateOperational &parentState, const OperationalState &newSubState);

        // --- Continuous Tick Handlers ---
        void onStateBootingTick(StateBooting &bootState);
        void onStateInitializationTick(StateInitializing &state);
        void onStateOperationalTick(StateOperational &opState);
        void onStatePreparePowerOffTick(const StatePreparePowerOff &state);

        // Template helper for executing basic services (Drawers, Stowage, Exposure)
        template <typename StateT>
        void onBasicOperatingServiceTick(StateT &state, Services::IService *service);

        // --- Pad/Static Processors ---
        void processCmdAlignmentPad(const CmdAlignmentPad &cmd);
        void processCmdZPad(const CmdZAxisPad &cmd);
        void processCmdVisionPad(const CmdVisionPad &cmd);

        // --- Safety Monitors ---
        void checkHardwareSafety(void);
        void stopAllServices(void);
        void updateAndBroadcastPosture(const ExpectedSystemPosture &expected, SystemPosture &current);

    private:
        SystemState m_state;                        // HFSM Macro-State (Single Source of Truth)
        Shared<HAL::MS::IMachineStatusRepo> m_repo; // The raw hardware values bus
        QTimer m_logicTimer;                        // 50Hz tick timer

        // Services (Pure Business Logic)
        Shared<Services::IHomingService> m_homingService;
        Shared<Services::IDrawerService> m_drawerService;
        Shared<Services::IStowageService> m_stowageService;
        Shared<Services::IAlignmentService> m_alignmentService;
        Shared<Services::IVisionService> m_visionService;
        Shared<Services::IContactService> m_contactService;
        Shared<Services::IExposureService> m_exposureService;
    };

} // namespace Kub3::MFSM
