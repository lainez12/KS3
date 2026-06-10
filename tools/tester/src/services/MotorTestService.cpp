#include <cmath>

#include <HAL/MachineStatus/utils.h>
#include <services/MotorTestService.h>

namespace Kub3::Tools::Tester
{

    MotorTestService::MotorTestService(Shared<HAL::Act::ActuatorRegistry> actuatorRegistry,
                                       std::vector<std::string> knownMotorIds) :
        m_actuatorRegistry(std::move(actuatorRegistry)),
        m_knownMotorIds(knownMotorIds),
        m_lastTickTime(std::chrono::steady_clock::now())
    {
        m_status = Services::ServiceStatus::Running;
    }

    // --- IService Implementation ---

    void MotorTestService::tick(void)
    {
        if (m_status != Services::ServiceStatus::Running)
            return;
        if (!m_selectedMotor.has_value())
            return;

        auto now  = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - m_lastTickTime).count();

        if (dt <= 0.001)
            return; // Prevent division by zero

        // Read Single Source of Truth
        const double currentPos = m_selectedMotor.value()->getEncoderPositionMm();
        // Finite difference calculations
        double currentSpeed = (currentPos - m_lastPosition) / dt;
        double currentAccel = (currentSpeed - m_lastSpeed) / dt;

        // Filter tiny noise out for clean UI (Deadband)
        if (std::abs(currentSpeed) < 0.0001)
            currentSpeed = 0.0;
        if (std::abs(currentAccel) < 0.0001)
            currentAccel = 0.0;

        // Update telemetry struct for the Owner to poll
        m_telemetry.positionMm = currentPos;
        m_telemetry.speedMmS   = currentSpeed;
        m_telemetry.accelMmS2  = currentAccel;

        m_lastPosition = currentPos;
        m_lastSpeed    = currentSpeed;
        m_lastTickTime = now;
    }

    void MotorTestService::stop(void)
    {
        emergencyStopAll();
        m_status = Services::ServiceStatus::Idle;
    }

    Services::ServiceStatus MotorTestService::getStatus(void) const noexcept
    {
        return m_status;
    }

    std::string MotorTestService::getErrorReason(void) const
    {
        return m_errorReason;
    }

    // --- Business Logic API ---

    std::vector<std::string> MotorTestService::getAvailableMotors() const
    {
        std::vector<std::string> motors;

        for (const auto &id : m_knownMotorIds)
        {
            if (m_actuatorRegistry->get<HAL::Act::IPositionMotor>(id).is_ok())
            {
                motors.push_back(id);
            }
        }
        return motors;
    }

    bool MotorTestService::selectMotor(const std::string &motorId)
    {
        auto motor = m_actuatorRegistry->get<HAL::Act::IPositionMotor>(motorId);

        if (motor.is_ok())
        {
            m_selectedMotor = motor.unwrap();

            // Reset kinematic history
            m_lastTickTime = std::chrono::steady_clock::now();
            m_lastPosition = motor->getEncoderPositionMm();
            m_lastSpeed    = 0.0;
            m_telemetry    = MotorTelemetry{m_lastPosition, 0.0, 0.0};

            return true;
        }

        qCritical().noquote().nospace() << "[MotorTestService] Failed to select motor " << motorId << ": " << motor.unwrap_err();
        m_selectedMotor = std::nullopt;
        return false;
    }

    void MotorTestService::startJog(HAL::Act::MotorDirection direction, const Config::kinematic_profile_t &profile)
    {
        if (m_selectedMotor.has_value())
        {
            m_selectedMotor.value()->moveDirection(direction, profile);
        }
    }

    void MotorTestService::stopJog()
    {
        if (m_selectedMotor.has_value())
        {
            m_selectedMotor.value()->emergencyStop();
        }
    }

    void MotorTestService::moveToAbsolute(double positionMm, const Config::kinematic_profile_t &profile)
    {
        if (m_selectedMotor.has_value())
        {
            m_selectedMotor.value()->moveAbsolute(positionMm, profile);
        }
    }

    void MotorTestService::emergencyStopAll()
    {
        for (const auto &id : m_knownMotorIds)
        {
            if (auto motor = m_actuatorRegistry->get<HAL::Act::IMotor>(id); motor)
                motor->emergencyStop();
        }
    }

} // namespace Kub3::Tools::Tester