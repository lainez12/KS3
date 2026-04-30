#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <MFSM/MasterFSM.h>

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
        checkHardwareSafety(); // Unconditional Safety Check (e.g. Physical Emergency-Stop Button)

        const auto museum = overloadedCallable(
            [&](StateBooting &bootState) { onStateBootingTick(bootState); },
            [&](StateWaitingInitialization &) { /* Wait for trigger */ },
            [&](StateInitialization &initState) { onStateInitializationTick(initState); },
            [&](StateIdle &) { /* Monitor temperatures, hold position */ },
            [&](StateOperating &operatingState) { onStateOperatingTick(operatingState); },
            [&](StateError &) { /* Blink red lights */ },
            [&](StateEmergencyStop &) { /* Ensure actuators are disabled */ },
            [&](StatePowerOff &powerOffState) { onStatePowerOffTick(powerOffState); });

        std::visit(museum, m_state); // Execute State-Specific Continuous Logic
    }

    void MasterFSM::checkHardwareSafety(void)
    {
        const bool emergencyStopTriggered  = HAL::MS::readBool(m_repo, EMERGENCY_STOP_BUTTON);
        const bool systemPowerOffTriggered = HAL::MS::readBool(m_repo, POWER_OFF_BUTTON);

        // Dispatch emergency stop event when:
        //  - Emergency stop press is detected
        //  - Not already in emergency stop state
        if (emergencyStopTriggered && !std::holds_alternative<StateEmergencyStop>(m_state))
        {
            dispatch(EvEmergencyStopTriggered{"Hardware Emergency Stop Button Pressed"});
        }
        if (systemPowerOffTriggered && !std::holds_alternative<StatePowerOff>(m_state))
        {
            dispatch(EvPowerOff{});
        }
    }

    void MasterFSM::onStateBootingTick(StateBooting &bootState)
    {
        bootState.ticksElapsed++;

        // (In the future, this list could be injected via ApplicationBuilder from config)
        static const std::vector<BootDependency> requiredDependencies = {
#if defined(KUB_MODEL_8)
            {MCU_ARDUINO1_ID, MCU_ARDUINO1_READY},
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

    void MasterFSM::onStateInitializationTick(StateInitialization &state)
    {
        m_homingService->tick();

        const Services::ServiceStatus status = m_homingService->getStatus();

        if (status == Services::ServiceStatus::Success)
            dispatch(EvInitializationComplete{});
        else if (status == Services::ServiceStatus::Error)
            dispatch(EvServiceError{.reason = m_homingService->getErrorReason()});
    }

    void MasterFSM::onStateOperatingTick(StateOperating &op)
    {
        const auto museum = overloadedCallable{
            [&](const Payloads::DrawerOpPayload &) { onBasicOperatingServiceTick(op, m_drawerService.get()); },
            [&](const Payloads::HomingOpPayload &) { onBasicOperatingServiceTick(op, m_homingService.get()); },
            [&](const Payloads::StowageOpPayload &) { onBasicOperatingServiceTick(op, m_stowageService.get()); },
            [&](const Payloads::ExposureOpPayload &) { onBasicOperatingServiceTick(op, m_exposureService.get()); },
            [&](const Payloads::AlignmentOpPayload &payload) {
                m_alignmentService->tick();
                m_visionService->tick();
                m_contactService->tick();
                // Watch contact to set hardware lock when contact is reached even when the contact phase is "free" (no contact)
                m_alignmentService->setHardwareLock(payload.phase != Payloads::ContactPhase::Free || m_contactService->isInContact());

                if (payload.phase == Payloads::ContactPhase::ApplyingContact ||
                    payload.phase == Payloads::ContactPhase::Separating)
                {
                    const Services::ServiceStatus zStatus = m_contactService->getStatus();

                    if (zStatus == Services::ServiceStatus::Success)
                        dispatch(EvContactSequenceComplete{});
                    else if (zStatus == Services::ServiceStatus::Error)
                        dispatch(EvServiceError{.reason = m_contactService->getErrorReason()});
                }
            },
        };

        std::visit(museum, op.payload);
    }

    void MasterFSM::onBasicOperatingServiceTick(StateOperating &op, Services::IService *service)
    {
        if (!service)
            return;
        service->tick();

        const Services::ServiceStatus status = service->getStatus();

        if (status == Services::ServiceStatus::Success)
            dispatch(EvServiceSuccess{});
        else if (status == Services::ServiceStatus::Error)
            dispatch(EvServiceError{.reason = service->getErrorReason()});
    }

    void MasterFSM::onStatePowerOffTick(StatePowerOff &powerOffState)
    {
        // TODO: Check homing state before powering off

        emit s_requestPowerOff();
    }

}
