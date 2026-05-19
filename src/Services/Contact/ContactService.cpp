#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Contact/ContactService.h>

namespace Kub3::Services
{

    // Ensures cast from an enum to another aligns with the values we want to provide
    static_assert(
        static_cast<int>(ZDirection::Up) == static_cast<int>(HAL::Act::MotorDirection::Positive) &&
            static_cast<int>(ZDirection::Down) == static_cast<int>(HAL::Act::MotorDirection::Negative),
        "ZDirection must align with HAL MotorDirection");

    ContactService::ContactService(Shared<HAL::Act::ActuatorRegistry> registry,
                                   Shared<HAL::MS::IMachineStatusRepo> repo,
                                   const Config::process_config_t &processConfig,
                                   const Config::hardware_config_t &hwConfig) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo))
    {
        m_zMotors = {
            m_registry->get<HAL::Act::IMotor>(Z_LEFT_MOTOR),
            m_registry->get<HAL::Act::IMotor>(Z_RIGHT_MOTOR),
            m_registry->get<HAL::Act::IMotor>(Z_BACK_MOTOR)};

        // Thresholds
        m_maxProcessForceGF   = processConfig.max_force_gf;            // Get absolute max force allowed to be requested
        m_hwCrashLimitForceGF = processConfig.hw_crash_force_limit_gf; // Get absolute max physical force allowed (hardware protection)
        m_contactThresholdGF  = processConfig.contact_threshold_gf;
        // Conversions factors
        if (auto it = hwConfig.adc_to_gf_factors.find(FORCE_LEFT); it != hwConfig.adc_to_gf_factors.end())
            m_adcToGFLeftFactor = it->second;
        else
            throw std::runtime_error("ContactService: Missing left sensor ADC to gram-force conversion factor in hardware configuration.");
        if (auto it = hwConfig.adc_to_gf_factors.find(FORCE_RIGHT); it != hwConfig.adc_to_gf_factors.end())
            m_adcToGFRightFactor = it->second;
        else
            throw std::runtime_error("ContactService: Missing right sensor ADC to gram-force conversion factor in hardware configuration.");
        if (auto it = hwConfig.adc_to_gf_factors.find(FORCE_BACK); it != hwConfig.adc_to_gf_factors.end())
            m_adcToGFBackFactor = it->second;
        else
            throw std::runtime_error("ContactService: Missing back sensor ADC to gram-force conversion factor in hardware configuration.");
    }

    void ContactService::tick(void)
    {
        // UNCONDITIONAL HARDWARE CRASH PROTECTION
        if (isHardwareCrashLimitExceeded())
        {
            this->stop(); // Kill watchdog and trigger Emergency Stop
            if (this->getStatus() == ServiceStatus::Running)
                abortSequence("CRITICAL: Absolute hardware crash limit exceeded during automation.");
            qCritical() << "CRITICAL: Absolute hardware crash limit exceeded. All Z motors emergency stopped.";
            return;
        }

        BaseTaskService::tick(); // Tick task lanes if any. If none, does nothing.

        // Tick the Manual Pad Watchdog
        if (m_manualWatchdogTicks > 0)
        {
            if (isProcessForceExceeded() && isMovingTowardsContact(m_currentManualDir))
            {
                this->stopZManual();
                qCritical() << "ContactService: Process force limit reached. Manual upward movement halted.";
                return;
            }

            m_manualWatchdogTicks--;
            if (m_manualWatchdogTicks == 0)
            {
                qWarning() << "ContactService: Dead-Man's switch triggered for Z motors.";
                for (auto &motor : m_zMotors)
                {
                    if (motor)
                        motor->emergencyStop();
                }
            }
        }
    }

    void ContactService::stop(void)
    {
        this->stopZManual();
        BaseTaskService::stop();
    }

    // ==========================================
    // PAD MOVEMENTS IMPLEMENTATIONS
    // ==========================================

    void ContactService::moveZManual(ZDirection dir)
    {
        // Not accepting pad movements while automated sequence is active
        if (this->getStatus() == ServiceStatus::Running)
        {
            qWarning() << "ContactService: Manual Z move rejected. Automated sequence is currently active.";
            return;
        }
        // Not accepting pad movements if max force is exceeded
        if (isProcessForceExceeded() && isMovingTowardsContact(dir))
        {
            qCritical() << "ContactService: Manual move rejected. Process force limit already exceeded.";
            return;
        }

        // Checking contact status to adapt kinematic profile
        m_currentManualDir  = dir;
        const auto &profile = isInContact() ? m_contactProfile : m_freeProfile;
        const auto halDir   = static_cast<HAL::Act::MotorDirection>(dir); // Ok to cast as values are the same

        if (m_manualWatchdogTicks == 0) // Start motors if they were stopped
        {
            for (auto &motor : m_zMotors)
            {
                if (motor)
                    motor->moveDirection(halDir, profile);
            }
        }

        // Reset the watchdog
        m_manualWatchdogTicks = Z_WATCHDOG_TIMEOUT_TICKS;
    }

    void ContactService::stopZManual(void)
    {
        m_manualWatchdogTicks = 0; // Disarm watchdog
        for (auto &motor : m_zMotors)
        {
            if (motor)
                motor->emergencyStop();
        }
    }

    // ==========================================
    // BASE TASK SERVICE IMPLEMENTATIONS
    // ==========================================

    void ContactService::startContactRoutine(ContactPayload kind)
    {
        this->clearTasks();

        bool setupSuccess = true;
        auto museum       = overloadedCallable(
            [&](const AutolevelingPayload &) { buildAutolevelingLanes(); },
            [&](const BasicContactPayload &p) {
                if (p.forceGF > m_maxProcessForceGF) {
                    abortSequence("Requested force exceeds maximum process limit.");
                    setupSuccess = false;
                    return;
                }
                buildBasicContactLanes(p.forceGF); },
            [&](const auto &) {});

        std::visit(museum, kind);
        if (setupSuccess)
            this->startSequence();
    }

    void ContactService::buildAutolevelingLanes(void)
    {
        // TODO: build lanes
    }

    void ContactService::buildBasicContactLanes(double forceGF)
    {
        // TODO: build lanes
    }

    // ==========================================
    // HARDWARE SENSOR EVALUATIONS
    // ==========================================

    // Helper to get max current force in gram-force
    double ContactService::getMaxCurrentForceGF() const
    {
        const double fL = static_cast<double>(HAL::MS::readUInt16(m_repo, FORCE_LEFT)) * m_adcToGFLeftFactor;
        const double fR = static_cast<double>(HAL::MS::readUInt16(m_repo, FORCE_RIGHT)) * m_adcToGFRightFactor;
        const double fB = static_cast<double>(HAL::MS::readUInt16(m_repo, FORCE_BACK)) * m_adcToGFBackFactor;

        return std::max({fL, fR, fB});
    }

    bool ContactService::isInContact(void) const
    {
        return getMaxCurrentForceGF() > m_contactThresholdGF;
    }

    bool ContactService::isProcessForceExceeded(void) const
    {
        return getMaxCurrentForceGF() > m_maxProcessForceGF;
    }

    bool ContactService::isHardwareCrashLimitExceeded(void) const
    {
        return getMaxCurrentForceGF() > m_hwCrashLimitForceGF;
    }

    inline constexpr bool ContactService::isMovingTowardsContact(ZDirection dir) const
    {
        return dir == ZDirection::Up;
    }

}
