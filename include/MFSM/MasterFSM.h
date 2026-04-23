#pragma once

#include <QObject>
#include <QRect>
#include <QString>
#include <QTimer>

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <MFSM/events.h>
#include <MFSM/states.h>
#include <Services/Drawers/IDrawerService.h>
#include <Services/Homing/IHomingService.h>

namespace Kub3::MFSM
{

    class MasterFSM final : public QObject
    {
        Q_OBJECT
    public:
        explicit MasterFSM(Shared<HAL::MS::IMachineStatusRepo> repo,
                           Shared<Services::IHomingService> homingService,
                           Shared<Services::IDrawerService> drawerService,
                           QObject *parent = nullptr);
        ~MasterFSM(void) override = default;

        // Starts the internal 50Hz logic loop
        void start(void);

    signals:
        // Tier 2 (Logic) -> Tier 1 (UI) Outputs
        void s_stateChanged(const QString &stateName);
        void s_errorOccurred(const QString &errorMessage);
        // Tier 2 (Logic) -> Tier 3 (I/O threads)
        void s_requestHardwareRetry(const QString &hardwareId);
        void s_requestPowerOff(void);
        void s_requestCameraParamUpdate(const QString &camId, HAL::Vision::CameraParamKind kind, HAL::Vision::CameraParam value);

    public slots:
        // Tier 1 (UI) -> Tier 2 (Logic) Thread-Safe Commands
        void ps_requestInitialization(void);
        void ps_requestOperateDrawer(int targetInt, int operationInt); // TODO: replace `int` type with enum or better for communication
        void ps_requestResetError(void);
        void ps_requestEmergencyStop(void); // unused for now
        void ps_systemPowerOff(void);       // unused for now
        void ps_requestExposureUpdate(const QString &camId, double val);
        void ps_requestGainUpdate(const QString &camId, double val);
        void ps_requestFrameRateUpdate(const QString &camId, double val);
        void ps_requestCenteredZoomUpdate(const QString &camId, double val);
        void ps_requestROIUpdate(const QString &camId, const QRect &roi);

    private slots:
        // The Heartbeat (50Hz)
        void onLogicTick(void);

        void onStateBootingTick(StateBooting &bootState);
        void onStateInitializationTick(StateInitialization &state);
        void onStateOperatingTick(StateOperating &operatingState);
        void onStatePowerOffTick(StatePowerOff &powerOffState);

    private:
        // Core FSM Methods
        void dispatch(const SystemEvent &event);
        [[nodiscard]] bool processStaticEvent(const SystemState &currentState, const SystemEvent &event);
        [[nodiscard]] SystemState processTransition(const SystemState &currentState, const SystemEvent &event);
        void onStateEntered(const SystemState &newState);
        // Safety Monitors
        void checkHardwareSafety();
        void stopAllServices();

    private:
        SystemState m_state;                        // Current Master FMS state
        Shared<HAL::MS::IMachineStatusRepo> m_repo; // TODO: define if necessary
        QTimer m_logicTimer;                        // Tick timer

        // Services
        Shared<Services::IHomingService> m_homingService; // Homing / Initialization Service
        Shared<Services::IDrawerService> m_drawerService; // Drawer Service
    };

} // namespace Kub3::MFSM
