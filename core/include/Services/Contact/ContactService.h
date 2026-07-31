#pragma once

#include <Algorithms/Admittance/AdmittanceController.h>
#include <Common/Enums.h>
#include <Common/TestToken.h>
#include <Config/conf.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/Actuators/Switches/ISwitch.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/BaseTaskService.h>
#include <Services/Contact/tasks/AdmittanceControlTask.h>
#include <utils.h>

#include "IContactService.h"

namespace Kub3::Services
{

    constexpr uint32_t Z_WATCHDOG_TIMEOUT_TICKS = 15u;

    struct AutolevelingPayload {};
    struct BasicContactPayload {
        double forceGF; // Force expressed in gram-force
    };
    struct HorizontalityPayload {};

    class ContactService final : public BaseTaskService<IContactService>
    {
    public:
        ContactService(Shared<HAL::Act::ActuatorRegistry> registry,
                       Shared<HAL::MS::IMachineStatusRepo> repo,
                       const Config::process_config_t &processConfig,
                       const Config::hardware_config_t &hwConfig);

        // IContactService overrides
        void startContactRoutine(ContactPayload kind) override;
        void retractFromContact(void) override;
        void moveZManual(ZDirection dir) override;
        void stopZManual(void) override;
        [[nodiscard]] bool isInContact(void) const override;
        void processBackgroundAutomations(void) override;

        // BaseTaskService overrides
        void tick(void) override;
        void onStop(void) override;

    public:
        // Test methods
        void tareForceSensor(TestToken, ForceSensor fs) override;
        void toggleForceSensors(TestToken, bool en) override;

    private:
        void _toggleForceSensors(bool en);
        void buildAutolevelingLanes(void);
        void buildBasicContactLanes(double forceGF);
        void buildHorizontalityLanes(void);
        Algorithms::Control::admittance_config_t buildAdmittanceConfig(double targetForceGF, double toleranceGF) const;

        void initializeMachineValues(void);
        void compensateTiltIfNeeded(void);

        // Internal helpers for contact status
        [[nodiscard]] double getMaxCurrentForceGF(void) const;
        [[nodiscard]] force_readings_t getCurrentForces(void) const;
        [[nodiscard]] bool isProcessForceExceeded(void) const;
        [[nodiscard]] bool isHardwareCrashLimitExceeded(void) const;
        [[nodiscard]] inline constexpr bool isMovingTowardsContact(ZDirection dir) const;

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Config::contact_process_config_t m_conf;

        std::array<Shared<HAL::Act::ISwitch>, 3> m_forceSensorsSw;
        std::array<Shared<HAL::Act::IPositionMotor>, 3> m_zMotors;
        uint32_t m_manualWatchdogTicks = 0;
        ZDirection m_currentManualDir  = ZDirection::Down;

        // Active Tilt Compensation trackers
        bool m_manualZPaused[3]  = {false, false, false};
        double m_manualStartZ[3] = {0.0, 0.0, 0.0};
        bool m_tiltWarningIssued = false;

        Config::kinematic_profile_t m_freeProfile;
        Config::kinematic_profile_t m_contactProfile;

        std::string m_taskAbortReason;

        double m_maxMotorsDeltaMm = 0.5;
        double m_requestedForceGF = 0.0;

        // Conversion factors (ADC to gram-force)
        double m_adcToGFLeftFactor                     = 1.0;
        double m_adcToGFRightFactor                    = 1.0;
        double m_adcToGFBackFactor                     = 1.0;
        Optional<plan_deltas_t> m_horizontalPlanDeltas = std::nullopt;
    };

}