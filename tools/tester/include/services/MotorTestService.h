#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include <Algorithms/Filters/TimeAwareEMAFilter.h>
#include <Config/conf.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/IService.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{
    // Pure C++ structure to hold polled telemetry
    struct MotorTelemetry {
        double positionMm = 0.0;
        double speedMmS   = 0.0;
        double accelMmS2  = 0.0;
    };

    class MotorTestService : public Services::IService
    {
    public:
        explicit MotorTestService(Shared<HAL::Act::ActuatorRegistry> actuatorRegistry,
                                  std::vector<std::string> knownMotorIds);

        MotorTestService(const MotorTestService &)            = delete;
        MotorTestService &operator=(const MotorTestService &) = delete;
        ~MotorTestService() override                          = default;

        // --- IService Implementation ---
        void tick(void) override;
        void stop(void) override;
        [[nodiscard]] Services::ServiceStatus getStatus(void) const noexcept override;
        [[nodiscard]] std::string getErrorReason(void) const override;

        // --- Business Logic API ---
        [[nodiscard]] std::vector<std::string> getAvailableMotors() const;
        bool selectMotor(const std::string &motorId);

        void startJog(HAL::Act::MotorDirection direction, const Config::kinematic_profile_t &profile);
        void stopJog(void);
        void moveToAbsolute(double positionMm, const Config::kinematic_profile_t &profile);
        void emergencyStopAll();

        // --- Telemetry & State Polling ---
        [[nodiscard]] bool hasValidMotor() const noexcept
        {
            return m_selectedMotor.has_value();
        }
        [[nodiscard]] MotorTelemetry getTelemetry() const noexcept
        {
            return m_telemetry;
        }
        [[nodiscard]] std::string_view getCurrentEncoderId(void) const
        {
            if (!m_selectedMotor.has_value())
                return "";
            return m_selectedMotor.value()->getEncoderId();
        }

    private:
        Shared<HAL::Act::ActuatorRegistry> m_actuatorRegistry;
        const std::vector<std::string> m_knownMotorIds;

        Services::ServiceStatus m_status = Services::ServiceStatus::Idle;
        std::string m_errorReason;

        // MISU: Hardware interactions are unrepresentable if this is nullopt
        std::optional<Shared<HAL::Act::IPositionMotor>> m_selectedMotor = std::nullopt;

        // Kinematic Calculation State
        std::chrono::steady_clock::time_point m_lastTickTime;
        double m_lastPosition      = 0.0;
        double m_lastFilteredSpeed = 0.0;

        MotorTelemetry m_telemetry;
        Algorithms::Filter::LowPassFilter m_speedFilter{10};
        Algorithms::Filter::LowPassFilter m_accelFilter{5};
    };

} // namespace Kub3::Tools::Tester
