#include <QDebug>

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>

#include <Services/Contact/ContactService.h>
#include <Services/Contact/tasks/AdmittanceControlTask.h>
#include <Services/Contact/tasks/FastApproachTask.h>
#include <Services/Contact/tasks/SaveCurrentPlanTask.h>

namespace Kub3::Services
{

    // Ensures cast from an enum to another aligns with the values we want to provide
    static_assert(
        static_cast<int>(ZDirection::Up) == static_cast<int>(HAL::Act::MotorDirection::Positive) &&
            static_cast<int>(ZDirection::Down) == static_cast<int>(HAL::Act::MotorDirection::Negative),
        "ZDirection must align with HAL MotorDirection");

    ContactService::ContactService(Shared<HAL::Act::ActuatorRegistry> registry,
                                   Shared<HAL::MS::IMachineStatusRepo> repo,
                                   const Config::process_config_t &processConf,
                                   const Config::hardware_config_t &hwConfig) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo)),
        m_conf(processConf.contact)
    {
        this->initializeMachineValues();

        // Force sensors switches
        UNWRAP_OR_THROW(leftForceSw, m_registry->get<HAL::Act::ISwitch>(FORCE_LEFT_SWITCH), "[ContactService] Failed to load Left Force Sensor Switch: ");
        UNWRAP_OR_THROW(rightForceSw, m_registry->get<HAL::Act::ISwitch>(FORCE_RIGHT_SWITCH), "[ContactService] Failed to load Right Force Sensor Switch: ");
        UNWRAP_OR_THROW(backForceSw, m_registry->get<HAL::Act::ISwitch>(FORCE_BACK_SWITCH), "[ContactService] Failed to load Back Force Sensor Switch: ");
        m_forceSensorsSw = {leftForceSw, rightForceSw, backForceSw};

        // Motors & kinematics
        UNWRAP_OR_THROW(leftMotor, m_registry->get<HAL::Act::IPositionMotor>(Z_LEFT_MOTOR), "[ContactService] Failed to load Z Left Motor: ");
        UNWRAP_OR_THROW(rightMotor, m_registry->get<HAL::Act::IPositionMotor>(Z_RIGHT_MOTOR), "[ContactService] Failed to load Z Right Motor: ");
        UNWRAP_OR_THROW(backMotor, m_registry->get<HAL::Act::IPositionMotor>(Z_BACK_MOTOR), "[ContactService] Failed to load Z Back Motor: ");
        m_zMotors          = {leftMotor, rightMotor, backMotor};
        m_freeProfile      = processConf.getKinematicProfile(Z_BACK_MOTOR, "normal");
        m_contactProfile   = processConf.getKinematicProfile(Z_BACK_MOTOR, "fine");
        m_maxMotorsDeltaMm = processConf.elevator.max_z_relative_distance_mm;

        // Conversions factors
        if (auto it = hwConfig.adc_to_gf_factors.find(FORCE_LEFT_ADC); it != hwConfig.adc_to_gf_factors.end())
            m_adcToGFLeftFactor = it->second;
        else
            throw std::runtime_error("ContactService: Missing left sensor ADC to gram-force conversion factor in hardware configuration.");

        if (auto it = hwConfig.adc_to_gf_factors.find(FORCE_RIGHT_ADC); it != hwConfig.adc_to_gf_factors.end())
            m_adcToGFRightFactor = it->second;
        else
            throw std::runtime_error("ContactService: Missing right sensor ADC to gram-force conversion factor in hardware configuration.");

        if (auto it = hwConfig.adc_to_gf_factors.find(FORCE_BACK_ADC); it != hwConfig.adc_to_gf_factors.end())
            m_adcToGFBackFactor = it->second;
        else
            throw std::runtime_error("ContactService: Missing back sensor ADC to gram-force conversion factor in hardware configuration.");

        qDebug().noquote() << QString("ADC to gram-force ratios (L ; R ; B): (%1 ; %2 ; %3)")
                                  .arg(m_adcToGFLeftFactor)
                                  .arg(m_adcToGFRightFactor)
                                  .arg(m_adcToGFBackFactor);
    }

    void ContactService::tick(void)
    {
        // Ensure no two Z-motors stray too far mechanically from each other.
        if (m_zMotors[0] && m_zMotors[1] && m_zMotors[2])
        {
            double zL = m_zMotors[0]->getEncoderPositionMm();
            double zR = m_zMotors[1]->getEncoderPositionMm();
            double zB = m_zMotors[2]->getEncoderPositionMm();

            // Motors delta maximum value (tilt limit) threshold detection
            if (std::abs(zL - zR) > m_maxMotorsDeltaMm ||
                std::abs(zR - zB) > m_maxMotorsDeltaMm ||
                std::abs(zL - zB) > m_maxMotorsDeltaMm)
            {
                this->stop();
                if (this->getStatus() == ServiceStatus::Running)
                    abortSequence("CRITICAL: Z-Motors relative distance exceeded max limit. Binding protection triggered.");

                qDebug() << QString("Positions: [L=%1; R=%2; B=%3]").arg(zL).arg(zR).arg(zB);
                qCritical() << "CRITICAL: Z-Motors binding protection triggered. Motors emergency stopped.";
                return;
            }
        }

        // UNCONDITIONAL HARDWARE CRASH PROTECTION
        if (isHardwareCrashLimitExceeded())
        {
            this->stop(); // Kill watchdog and trigger Emergency Stop
            if (this->getStatus() == ServiceStatus::Running)
                abortSequence("CRITICAL: Absolute hardware crash limit exceeded during automation.");
            qCritical() << "CRITICAL: Absolute hardware crash limit exceeded. All Z motors emergency stopped.";
            return;
        }

        if (!m_taskAbortReason.empty())
        {
            this->abortSequence(m_taskAbortReason);
            m_taskAbortReason.clear();
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

    void ContactService::onStop(void)
    {
        this->stopZManual();
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

    void ContactService::toggleForceSensors(bool en)
    {
        for (auto sw : m_forceSensorsSw)
        {
            if (!sw)
                continue;

            if (en)
                sw->turnOn();
            else
                sw->turnOff();
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
                if (p.forceGF > m_conf.max_process_force_gf) {
                    abortSequence("Requested force exceeds maximum process limit.");
                    setupSuccess = false;
                    return;
                }
                buildBasicContactLanes(p.forceGF); },
            [&](const HorizontalityPayload &) {
                if (!m_horizontalPlanDeltas.has_value()) {
                    abortSequence("No horizontal plan was saved.");
                    setupSuccess = false;
                    return;
                }
                buildHorizontalityLanes(); },
            [&](const auto &) {});

        std::visit(museum, kind);
        if (setupSuccess)
        {
            this->toggleForceSensors(true);
            this->startSequence();
        }
    }

    void ContactService::buildAutolevelingLanes(void)
    {
        auto abortCb        = [this](std::string reason) { m_taskAbortReason = std::move(reason); };
        auto maxForceGetter = [this]() -> double { return this->getMaxCurrentForceGF(); };
        auto forceGetter    = [this]() -> force_readings_t { return this->getCurrentForces(); };

        // Approach until contact threshold is touched.
        this->enqueueTask<FastApproachTask>(m_zMotors, maxForceGetter, m_conf.contact_threshold_gf, m_freeProfile);
        // Climb and planarize at the target point
        this->enqueueTask<AdmittanceControlTask>(m_zMotors, forceGetter, abortCb,
                                                 buildAdmittanceConfig(m_conf.autolevel_force_gf, m_conf.autolevel_force_tolerance_gf),
                                                 AdmittanceControlTask::Mode::Autoleveling,
                                                 m_contactProfile);
        // Save relative deltas of the reached planeity
        this->enqueueTask<SaveCurrentPlanTask>(m_zMotors, m_horizontalPlanDeltas);
    }

    void ContactService::buildBasicContactLanes(double forceGF)
    {
        auto abortCb        = [this](std::string reason) { m_taskAbortReason = std::move(reason); };
        auto maxForceGetter = [this]() -> double { return this->getMaxCurrentForceGF(); };
        auto forceGetter    = [this]() -> force_readings_t { return this->getCurrentForces(); };

        // Approach until contact threshold is touched.
        this->enqueueTask<FastApproachTask>(m_zMotors, maxForceGetter, m_conf.contact_threshold_gf, m_freeProfile);
        // Synchronous movement up to target force.
        this->enqueueTask<AdmittanceControlTask>(m_zMotors, forceGetter, abortCb,
                                                 buildAdmittanceConfig(forceGF, m_conf.autolevel_force_tolerance_gf), // TODO: define another tolerance threshold
                                                 AdmittanceControlTask::Mode::BasicContact,
                                                 m_contactProfile);
    }

    void ContactService::buildHorizontalityLanes(void)
    {
        // TODO: build lanes to perform horizontal planeity
    }

    Algorithms::Control::admittance_config_t ContactService::buildAdmittanceConfig(double targetForceGF, double toleranceGF) const
    {
        qDebug().noquote() << "[Admittance configuration] Target force:" << targetForceGF << "gF, Tolerance" << toleranceGF << "gf, Deadband V:" << m_conf.admittance.deadband_velocity_mm_s << "mm/s";
        return Algorithms::Control::admittance_config_t{
            // Targets and Limits
            .target_force_gf      = targetForceGF,
            .force_tolerance_gf   = toleranceGF,
            .max_process_force_gf = m_conf.max_process_force_gf,

            // Gain Scheduling (Compliance: mm/s per GF)
            .k_mean_max = m_conf.admittance.translational_gain_low_force,  // Fast approach (Soft)
            .k_mean_min = m_conf.admittance.translational_gain_high_force, // Slow approach (Stiff/Contact)
            .k_tilt_max = m_conf.admittance.rotational_gain_low_force,     // WEC twist fast
            .k_tilt_min = m_conf.admittance.rotational_gain_high_force,    // WEC twist slow

            // Safety Limits & Hardware capabilities
            .max_step_mm_per_tick   = m_conf.admittance.max_step_mm_per_tick,   // Max allowed blind travel per tick
            .max_profile_speed_mm_s = m_contactProfile.targetVelocityMmS,       // Max allowed continuous speed
            .deadband_velocity_mm_s = m_conf.admittance.deadband_velocity_mm_s, // Hardware deadband limit (to prevent stuttering)
        };
    }

    void ContactService::initializeMachineValues(void)
    {
        m_repo->setValueRaw(V_LEFT_Z_HORIZONTALITY_DELTA, 0);
        m_repo->setValueRaw(V_RIGHT_Z_HORIZONTALITY_DELTA, 0);
        m_repo->setValueRaw(V_BACK_Z_HORIZONTALITY_DELTA, 0);
    }

    // ==========================================
    // HARDWARE SENSOR EVALUATIONS
    // ==========================================

    // Helper to get max current force in gram-force
    double ContactService::getMaxCurrentForceGF() const
    {
        const uint16_t adcFL     = HAL::MS::readUInt16(m_repo, FORCE_LEFT_ADC);
        const uint16_t adcFR     = HAL::MS::readUInt16(m_repo, FORCE_RIGHT_ADC);
        const uint16_t adcFB     = HAL::MS::readUInt16(m_repo, FORCE_BACK_ADC);
        const uint16_t adcFTareL = HAL::MS::readUInt16(m_repo, V_TARE_FORCE_LEFT_ADC);
        const uint16_t adcFTareR = HAL::MS::readUInt16(m_repo, V_TARE_FORCE_RIGHT_ADC);
        const uint16_t adcFTareB = HAL::MS::readUInt16(m_repo, V_TARE_FORCE_BACK_ADC);

        const double fL = static_cast<double>(adcFL - adcFTareL) * m_adcToGFLeftFactor;
        const double fR = static_cast<double>(adcFR - adcFTareR) * m_adcToGFRightFactor;
        const double fB = static_cast<double>(adcFB - adcFTareB) * m_adcToGFBackFactor;

        return std::max({fL, fR, fB});
    }

    force_readings_t ContactService::getCurrentForces(void) const
    {
        const uint16_t adcFL     = HAL::MS::readUInt16(m_repo, FORCE_LEFT_ADC);
        const uint16_t adcFR     = HAL::MS::readUInt16(m_repo, FORCE_RIGHT_ADC);
        const uint16_t adcFB     = HAL::MS::readUInt16(m_repo, FORCE_BACK_ADC);
        const uint16_t adcFTareL = HAL::MS::readUInt16(m_repo, V_TARE_FORCE_LEFT_ADC);
        const uint16_t adcFTareR = HAL::MS::readUInt16(m_repo, V_TARE_FORCE_RIGHT_ADC);
        const uint16_t adcFTareB = HAL::MS::readUInt16(m_repo, V_TARE_FORCE_BACK_ADC);

        const double fL = static_cast<double>(adcFL - adcFTareL) * m_adcToGFLeftFactor;
        const double fR = static_cast<double>(adcFR - adcFTareR) * m_adcToGFRightFactor;
        const double fB = static_cast<double>(adcFB - adcFTareB) * m_adcToGFBackFactor;

        qDebug() << QString("(Current forces) LEFT = %1 (%2 gF, tare: %3); RIGHT = %7 (%8 gF, tare: %9); BACK = %4 (%5 gF, tare: %6)")
                        .arg(adcFL - adcFTareL)
                        .arg(fL)
                        .arg(adcFTareL)
                        .arg(adcFR - adcFTareR)
                        .arg(fR)
                        .arg(adcFTareR)
                        .arg(adcFB - adcFTareB)
                        .arg(fB)
                        .arg(adcFTareB);

        return {fL, fR, fB, std::max({fL, fR, fB})};
    }

    bool ContactService::isInContact(void) const
    {
        return getMaxCurrentForceGF() > m_conf.contact_threshold_gf;
    }

    bool ContactService::isProcessForceExceeded(void) const
    {
        return getMaxCurrentForceGF() > m_conf.max_process_force_gf;
    }

    bool ContactService::isHardwareCrashLimitExceeded(void) const
    {
        return getMaxCurrentForceGF() > m_conf.hw_crash_force_limit_gf;
    }

    inline constexpr bool ContactService::isMovingTowardsContact(ZDirection dir) const
    {
        return dir == ZDirection::Up;
    }

} // namespace Kub3::Services
