#include <QDebug>

#include "HAL/MachineStatus/sensors_labels.h"
#include "HAL/MachineStatus/utils.h"
#include "MFSM/MasterFSM.h"

namespace
{
    struct BootDependency {
        std::string subsystemId;
        std::string readyKey;
    };
}

namespace Kub3::MFSM
{

    MasterFSM::MasterFSM(Shared<HAL::MS::IMachineStatusRepo> repo,
                         Shared<Services::IDrawerService> drawerService,
                         QObject *parent) :
        QObject(parent),
        m_state(StateBooting{}),
        m_repo(std::move(repo)),
        m_drawerService(std::move(drawerService)),
        m_logicTimer(this)
    {
        connect(&m_logicTimer, &QTimer::timeout, this, &MasterFSM::onLogicTick);
    }

    void MasterFSM::start(void)
    {
        m_logicTimer.start(20);         // 50Hz
        emit s_stateChanged("BOOTING"); // Signal the initial state to the UI
    }

    // ==========================================
    // HEARTBEAT LOGIC
    // ==========================================

    void MasterFSM::onLogicTick(void)
    {
        checkHardwareSafety(); // Unconditional Safety Check (e.g. Physical E-Stop Button)

        const auto visitor = overloadedCallable(
            [&](StateBooting &bootState) { onStateBootingTick(bootState); },
            [&](StateWaitingInitialization &) { /* Wait for trigger */ },
            [&](StateInitialization &) { /* Tick homing service */ },
            [&](StateIdle &) { /* Monitor temperatures, hold position */ },
            [&](StateOperating &operatingState) { onStateOperatingTick(operatingState); },
            [&](StateError &) { /* Blink red lights */ },
            [&](StateEmergencyStop &) { /* Ensure motors are disabled */ });

        std::visit(visitor, m_state); // Execute State-Specific Continuous Logic
    }

    void MasterFSM::onStateBootingTick(StateBooting &bootState)
    {
        bootState.ticksElapsed++;

        // (In the future, this list could be injected via ApplicationBuilder from config)
        static const std::vector<BootDependency> requiredDependencies = {
#if defined(KUB_MODEL_8)
            {MCU_ARDUINO2_ID, MCU_ARDUINO2_READY},
            {MCU_ARDUINO3_ID, MCU_ARDUINO3_READY}
#endif
        };

        constexpr uint32_t TICKS_PER_SEC = 50;
        constexpr uint32_t TIMEOUT_TICKS = 5 * TICKS_PER_SEC; // 5 seconds
        constexpr uint8_t MAX_RETRIES    = 3;

        bool allReady             = true;
        const bool timeoutReached = (bootState.ticksElapsed >= TIMEOUT_TICKS);
        bool fatalFailure         = false;
        std::vector<std::string> missingDependencies;

        // TODO: improve to have all missing dependencies messages when only 1 reaches its max retries
        for (const auto &dep : requiredDependencies)
        {
            if (!HAL::MS::readBool(m_repo, dep.readyKey))
            {
                allReady = false;
                missingDependencies.push_back(dep.subsystemId);

                // Timeout & Retry Logic
                if (timeoutReached)
                {
                    if (bootState.retryCounts[dep.subsystemId] < MAX_RETRIES)
                    {
                        bootState.retryCounts[dep.subsystemId]++;
                        qWarning() << std::format("MFSM: Timeout on {}. Requesting targeted retry ({}/{})", dep.subsystemId, bootState.retryCounts[dep.subsystemId], MAX_RETRIES);
                        emit s_requestHardwareRetry(QString::fromStdString(dep.subsystemId)); // Route the restart request for this specific subsystem
                    }
                    else
                    {
                        fatalFailure = true;
                    }
                }
            }
        }

        if (!missingDependencies.empty())
        {
            // Joining error strings with ", " separator
            std::string errorMsg = "Hardware timeout on: " + missingDependencies.front();

            errorMsg = std::accumulate(
                std::next(missingDependencies.begin()),
                missingDependencies.end(),
                errorMsg,
                [](const std::string &a, const std::string &b) { return std::move(a) + ", " + std::move(b); });

            qCritical() << std::format("MFSM: Max retries reached. {}", errorMsg).c_str();
            dispatch(EvHardwareError{errorMsg});
            return;
        }

        if (allReady)
            dispatch(EvHardwareReady{});
        else if (timeoutReached)        // Timeout & not all ready
            bootState.ticksElapsed = 0; // Reset timer for the next retry window
    }

    void MasterFSM::onStateOperatingTick(StateOperating &op)
    {
        Services::IService *activeService = nullptr;

        std::visit(
            overloadedCallable{
                [&](const Payloads::DrawerOpPayload &) { activeService = m_drawerService.get(); }
                // TODO: Add more
            },
            op.payload);

        if (!activeService)
            return;
        activeService->tick(); // Tick the active service

        const Services::ServiceStatus status = activeService->getStatus();

        if (status == Services::ServiceStatus::Success)
            dispatch(EvServiceSuccess{});
        else if (status == Services::ServiceStatus::Error)
            dispatch(EvServiceError{.reason = activeService->getErrorReason()});
    }

    void MasterFSM::checkHardwareSafety(void)
    {
        const bool emergencyStopTriggered = HAL::MS::readBool(m_repo, EMERGENCY_STOP);

        // Dispatch emergency stop event when:
        //  - Emergency stop press is detected
        //  - Not already in emergency stop state
        if (emergencyStopTriggered && !std::holds_alternative<StateEmergencyStop>(m_state))
        {
            dispatch(EvEmergencyStopTriggered{"Hardware Emergency Stop Button Pressed"});
        }
    }

    // ==========================================
    // UI SLOTS (User Inputs)
    // ==========================================

    void MasterFSM::ps_requestInitialization(void)
    {
        dispatch(CmdStartInitialization{});
    }

    void MasterFSM::ps_requestOperateDrawer(int targetInt, int operationInt)
    {
        auto target = static_cast<Services::DrawerTarget>(targetInt);
        auto op     = static_cast<DrawerOperation>(operationInt);

        dispatch(CmdOperateDrawer{target});
    }

    void MasterFSM::ps_requestResetError(void)
    {
        dispatch(CmdResetError{});
    }

    void MasterFSM::ps_requestEmergencyStop(void)
    {
        dispatch(EvEmergencyStopTriggered{"Software E-Stop Triggered by Operator"});
    }

    // ==========================================
    // FSM TRANSITION DISPATCHER
    // ==========================================

    void MasterFSM::dispatch(const SystemEvent &event)
    {
        // 1. Calculate the next state based on current state + event
        SystemState nextState = processTransition(m_state, event);

        // 2. If the state changed, apply it and trigger entry actions
        if (nextState.index() != m_state.index())
        {
            m_state = nextState;
            onStateEntered(m_state);
        }
    }

    // Mathematically pure function mapping: (State x Event) -> State
    SystemState MasterFSM::processTransition(const SystemState &currentState, const SystemEvent &event)
    {
        const auto visitor = overloadedCallable(
            // Global Event Overrides (Can happen in almost any state)
            [&](const auto &, const EvEmergencyStopTriggered &e) -> SystemState { return StateEmergencyStop{e.reason}; },

            // Booting -> Waiting for initialization
            [&](const StateBooting &, const EvHardwareReady &) -> SystemState { return StateWaitingInitialization{}; },

            // Booting -> Error
            [&](const StateBooting &, const EvHardwareError &e) -> SystemState { return StateError{e.reason}; },

            // Waiting for initialization -> Initialization
            [&](const StateWaitingInitialization &, const CmdStartInitialization &) -> SystemState { return StateInitialization{}; },

            // Initialization -> Idle
            [&](const StateInitialization &, const EvInitializationComplete &) -> SystemState { return StateIdle{}; },

            // Idle -> Operating Drawer (Insert/Eject)
            [&](const StateIdle &, const CmdOperateDrawer &cmd) -> SystemState {
                const Payloads::DrawerOpPayload payload{
                    .kind   = cmd.operation,
                    .target = cmd.target,
                };
                return StateOperating{payload};
            },

            // Operating -> Idle
            [&](const StateOperating &, const EvServiceSuccess &c) -> SystemState { return StateIdle{}; },

            // Operating -> Error
            [&](const StateOperating &, const EvServiceError &e) -> SystemState { return StateError{e.reason}; },

            // Error -> Idle (Reset)
            [&](const StateError &, const CmdResetError &) -> SystemState { return StateIdle{}; },

            // Fallback: If an event is not handled for the current state, remain in current state.
            [&](const auto &, const auto &) -> SystemState {
                qWarning() << "MFSM: Ignored Event in current state.";
                return currentState;
            });

        return std::visit(visitor, currentState, event);
    }

    // Actions executed exactly ONCE upon entering a state
    void MasterFSM::onStateEntered(const SystemState &newState)
    {
        // 1. We ALWAYS send a string to the UI so it can update its visual state machine
        const auto nameExtractor = overloadedCallable{
            [](const StateBooting &) { return QString("BOOTING"); },
            [](const StateWaitingInitialization &) { return QString("WAITING_INITIALIZATION"); },
            [](const StateInitialization &) { return QString("INITIALIZING"); },
            [](const StateIdle &) { return QString("IDLE"); },
            [](const StateOperating &) { return QString("OPERATING"); },
            [](const StateError &) { return QString("ERROR"); },
            [](const StateEmergencyStop &) { return QString("EMERGENCY_STOP"); }};

        emit s_stateChanged(std::visit(nameExtractor, newState));

        // 2. Perform Physical State Entry Actions
        const auto entryActions = overloadedCallable{
            [&](const StateBooting &) { /* no-op */ },

            [&](const StateWaitingInitialization &) { /* no-op */ },

            [&](const StateInitialization &) {
                // Fake initialization for now
                QTimer::singleShot(1500, this, [this]() { dispatch(EvInitializationComplete{}); });
            },

            [&](const StateIdle &) {
                // Halt all services when entering idle
                m_drawerService->stop();
            },

            [&](const StateOperating &s) {
                std::visit(
                    overloadedCallable{
                        [&](const Payloads::DrawerOpPayload &payload) {
                            if (payload.kind == DrawerOperation::EJECT)
                                m_drawerService->eject(payload.target);
                            else if (payload.kind == DrawerOperation::INSERT)
                                m_drawerService->insert(payload.target);
                        }},
                    s.payload);
            },

            [&](const StateError &s) {
                emit s_errorOccurred(QString::fromStdString(s.message));
                m_drawerService->stop();
            },

            [&](const StateEmergencyStop &s) {
                emit s_errorOccurred(QString::fromStdString(s.reason));
                m_drawerService->stop(); // Force all motors to emergency stop
            }};

        std::visit(entryActions, newState);
    }

} // namespace Kub3::MFSM
