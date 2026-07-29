#include <QDebug>

#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <MFSM/MasterFSM.h>
#include <utils.h>

namespace
{
    struct BootDependency {
        std::string subsystemId;
        std::string readyKey;
    };
}

namespace Kub3::MFSM
{

    // ==========================================
    // HEARTBEAT LOGIC
    // ==========================================

    void MasterFSM::onLogicTick(void)
    {
        // Unconditional Safety Check (Physical Emergency-Stop and PowerOff Buttons)
        this->checkHardwareSafety();

        const bool isSystemInitialized = !std::holds_alternative<StateBooting>(m_state) &&
                                         !std::holds_alternative<StateWaitingInitialization>(m_state) &&
                                         !std::holds_alternative<StateInitializing>(m_state);

        // Execute continuous hardware rules
        if (isSystemInitialized)
        {
            m_contactService->processBackgroundAutomations(); // Enables force sensors when necessary
        }

        // 2. Route the tick to the current active Macro-State
        const auto museum = overloadedCallable{
            [&](StateBooting &s) { this->onStateBootingTick(s); },
            [&](StateInitializing &s) { this->onStateInitializationTick(s); },
            [&](StateOperational &s) { this->onStateOperationalTick(s); },
            [&](StatePreparePowerOff &s) { this->onStatePreparePowerOffTick(s); },
            // Passive states that only respond to external events (No active ticking)
            [&](auto &) { /* no-op */ }};

        std::visit(museum, m_state);
    }

    // ==============================================
    // MACRO-STATE TICKS
    // ==============================================

    void MasterFSM::onStateBootingTick(StateBooting &bootState)
    {
        bootState.ticksElapsed++;

        static const std::vector<BootDependency> requiredDependencies = {
#if defined(KUB_MODEL_8)
        // {MCU_ARDUINO1_ID, MCU_ARDUINO1_READY},
        // {MCU_ARDUINO2_ID, MCU_ARDUINO2_READY},
        // {MCU_ARDUINO3_ID, MCU_ARDUINO3_READY}
#endif
        };

        constexpr uint32_t TICKS_PER_SEC = 1000u / LOGIC_TIMER_PERIOD_MS;
        constexpr uint32_t TIMEOUT_TICKS = 5 * TICKS_PER_SEC; // 5 seconds
        constexpr uint8_t MAX_RETRIES    = 3;

        bool allReady             = true;
        const bool timeoutReached = (bootState.ticksElapsed >= TIMEOUT_TICKS);
        bool fatalFailure         = false;
        std::vector<std::string> missingDependencies;

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

        if (fatalFailure)
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

    void MasterFSM::onStateInitializationTick(StateInitializing &)
    {
        m_homingService->tick();
        const Services::ServiceStatus status = m_homingService->getStatus();

        if (status == Services::ServiceStatus::Error)
            dispatch(EvServiceError{.reason = m_homingService->getErrorReason()});
        else if (status == Services::ServiceStatus::Success)
        {
            dispatch(EvInitializationComplete{});
            emit s_initializationSuccess();
        }
    }

    void MasterFSM::onStatePreparePowerOffTick(const StatePreparePowerOff &s)
    {
        m_homingService->tick();
        const Services::ServiceStatus status = m_homingService->getStatus();

        if (status == Services::ServiceStatus::Success)
            dispatch(EvPowerOff{});
        else if (status == Services::ServiceStatus::Error)
        {
            qCritical().nospace()
                << "Failed to perform homing procedure before powering off: " << m_homingService->getErrorReason()
                << "\nForcing shutdown.";
            dispatch(EvPowerOff{}); // Force shutdown anyway
        }
    }

    // ==========================================================================
    // OPERATIONAL SUB-STATE TICKS (Level 2)
    // ==========================================================================
    void MasterFSM::onStateOperationalTick(StateOperational &opState)
    {
        const auto museum = overloadedCallable{
            [&](StateIdle &) { /* no-op */ },
            [&](StateDrawerOp &s) { onBasicOperatingServiceTick(s, m_drawerService.get()); },
            [&](StateStowing &s) { onBasicOperatingServiceTick(s, m_stowageService.get()); },
            [&](StateUnstowing &s) { onBasicOperatingServiceTick(s, m_homingService.get()); },
            [&](StateAutoleveling &s) { onBasicOperatingServiceTick(s, m_contactService.get()); },
            [&](StateRetractingZ &s) { onBasicOperatingServiceTick(s, m_contactService.get()); },
            [&](StatePreparingAlignment &s) { onBasicOperatingServiceTick(s, m_visionService.get()); },
            [&](StateAlignment &s) {
                // Alignment mode is highly interactive: multiple services tick simultaneously.
                m_alignmentService->tick();
                m_visionService->tick();
                m_contactService->tick();
                // Dynamic Hardware Lock: Alignment axes must be frozen if contact is applying/active
                m_alignmentService->setHardwareLock((s.phase != ContactPhase::Free || m_contactService->isInContact()));

                // Check for sequence completions if we are moving the Z stage
                if (s.phase == ContactPhase::ApplyingContact || s.phase == ContactPhase::Separating)
                {
                    const auto zStatus = m_contactService->getStatus();
                    if (zStatus == Services::ServiceStatus::Success)
                        dispatch(EvContactSequenceComplete{});
                    else if (zStatus == Services::ServiceStatus::Error)
                        dispatch(EvServiceError{.reason = m_contactService->getErrorReason()});
                }
            },
            [&](StatePreparingExposure &s) { onBasicOperatingServiceTick(s, m_homingService.get()); }, // `HomingService` is used here to move vision block to home
            [&](StateExposureReady &s) { /* no-op */ },
            [&](StateExposing &s) { onBasicOperatingServiceTick(s, m_exposureService.get()); }};

        std::visit(museum, opState.subState);
    }

    // ==========================================================================
    // GENERIC SERVICE TICKER & POSTURE MERGER
    // ==========================================================================
    template <typename StateT>
    void MasterFSM::onBasicOperatingServiceTick(StateT &state, Services::IService *service)
    {
        if (!service)
            return;
        service->tick();

        const Services::ServiceStatus status = service->getStatus();

        if (status == Services::ServiceStatus::Success)
        {
            // C++20 SFINAE/Concepts-lite magic:
            // If this particular sub-state struct has an 'expectedSuccess' posture payload,
            // merge it into the global SystemPosture before signaling success!
            if constexpr (requires { state.expectedSuccess; })
            {
                if (auto *opState = std::get_if<StateOperational>(&m_state))
                    this->updateAndBroadcastPosture(state.expectedSuccess, opState->posture);
            }

            dispatch(EvServiceSuccess{});
        }
        else if (status == Services::ServiceStatus::Error)
        {
            dispatch(EvServiceError{.reason = service->getErrorReason()});
        }
    }

} // namespace Kub3::MFSM
