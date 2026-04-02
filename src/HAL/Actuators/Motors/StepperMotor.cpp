#include <QDebug>
#include <QMetaObject>
#include <stdexcept>

#include "HAL/Actuators/Motors/StepperMotor.h"

#define CONTROL_TIMER_VALUE_MS 20 // 50Hz

namespace Kub3::HAL::Act
{

    StepperMotor::StepperMotor(std::string id,
                               uint8_t byteId,
                               Weak<MCUDriver> driver,
                               Config::stepper_hw_properties_t hwConfig,
                               std::function<double()> posGetter,
                               QObject *parent) :
        QObject(parent),
        m_id(std::move(id)),
        m_byteId(byteId),
        m_hwConfig(std::move(hwConfig)),
        m_driver(std::move(driver)),
        m_positionGetter(std::move(posGetter)),
        m_controlTimer(this)
    {
        qDebug() << std::format("Stepper[{}] Loaded config:", m_id);
        qDebug() << std::format("--- stepsPerRev={}", m_hwConfig.stepsPerRev);
        qDebug() << std::format("--- screwPitchMm={}", m_hwConfig.screwPitchMm);
        qDebug() << std::format("--- maxVelocityMmS={}", m_hwConfig.maxVelocityMmS);
        qDebug() << std::format("--- maxAccelerationMmS2={}", m_hwConfig.maxAccelerationMmS2);

        connect(&m_controlTimer, &QTimer::timeout, this, &StepperMotor::onControlTick);
    }

    void StepperMotor::sendPayload(const uint8_t *payload, uint32_t size) const
    {
        if (auto driver = m_driver.lock())
        {
            QMetaObject::invokeMethod(
                driver.get(),
                &MCUDriver::ps_sendCommand,
                Qt::QueuedConnection,
                QByteArray(reinterpret_cast<const char *>(payload), size));
        }
        else
        {
            throw std::runtime_error("Attempted to send command, but MCUDriver is dead. Actuator: " + m_id);
        }
    }

    void StepperMotor::moveAbsolute(double position_mm, Config::kinematic_profile_t profile)
    {
        // Clamp to safety limits
        double safeVel = std::min(profile.targetVelocityMmS, m_hwConfig.maxVelocityMmS);
        double safeAcc = std::min(profile.accelerationMmS2, m_hwConfig.maxAccelerationMmS2);

        // Extract step fraction safely
        uint8_t stepFrac = 1;
        if (auto *p = std::get_if<Config::stepper_kinematics_params_t>(&profile.params))
            stepFrac = p->stepFraction;

        m_currentStepFraction = stepFrac; // Store for MCU translation
        m_lastSentHz.reset();             // Reset cached "last sent frequency" value;

        if (!m_controlTimer.isActive()) // Start the control timer
        {
            // Initialize the math engine
            m_mathEngine.startPositionMove(getEncoderPositionMm(), position_mm, safeVel, safeAcc);

            m_lastTickNsecs = 0; // Reset last tick timestamp
            m_dtTimer.start();   // reset elapsed timer
            m_controlTimer.start(CONTROL_TIMER_VALUE_MS);
        }
        else // Motor already moving
        {
            // Update the math engine
            m_mathEngine.updatePositionMove(position_mm, safeVel, safeAcc);
        }
    }

    void StepperMotor::moveRelative(double distance_mm, Config::kinematic_profile_t profile)
    {
        // Clamp to safety limits
        double safeVel = std::min(profile.targetVelocityMmS, m_hwConfig.maxVelocityMmS);
        double safeAcc = std::min(profile.accelerationMmS2, m_hwConfig.maxAccelerationMmS2);

        // Extract step fraction safely
        uint8_t stepFrac = 1;
        if (auto *p = std::get_if<Config::stepper_kinematics_params_t>(&profile.params))
            stepFrac = p->stepFraction;

        m_currentStepFraction = stepFrac; // Store for MCU translation
        m_lastSentHz.reset();             // Reset cached "last sent frequency" value;

        if (!m_controlTimer.isActive()) // Start the control timer
        {
            const double encoderPos = getEncoderPositionMm();

            // Initialize the math engine
            m_mathEngine.startPositionMove(encoderPos, encoderPos + distance_mm, safeVel, safeAcc);

            m_lastTickNsecs = 0; // Reset last tick timestamp
            m_dtTimer.start();   // reset elapsed timer
            m_controlTimer.start(CONTROL_TIMER_VALUE_MS);
        }
        else // Motor already moving
        {
            const double currentMathEnginePos = m_mathEngine.getCurrentState().position;

            // Update the math engine
            m_mathEngine.updatePositionMove(currentMathEnginePos + distance_mm, safeVel, safeAcc);
        }
    }

    void StepperMotor::moveDirection(MotorDirection dir, Config::kinematic_profile_t profile)
    {
        // Clamp to safety limits
        double safeVel = std::min(profile.targetVelocityMmS, m_hwConfig.maxVelocityMmS);
        double safeAcc = std::min(profile.accelerationMmS2, m_hwConfig.maxAccelerationMmS2);

        // Extract step fraction safely
        uint16_t stepFrac = 1;
        if (auto *p = std::get_if<Config::stepper_kinematics_params_t>(&profile.params))
            stepFrac = p->stepFraction;

        m_currentStepFraction = stepFrac; // Store for MCU translation
        m_lastSentHz.reset();             // Reset cached "last sent frequency" value;

        const double sign = (dir == MotorDirection::Positive) ? 1.0 : -1.0;

        if (!m_controlTimer.isActive()) // Start the control timer
        {
            // Initialize the pure math engine
            m_mathEngine.startVelocityMove(getEncoderPositionMm(), sign, safeVel, safeAcc);

            m_lastTickNsecs = 0; // Reset last tick timestamp
            m_dtTimer.start();   // reset elapsed timer
            m_controlTimer.start(CONTROL_TIMER_VALUE_MS);
        }
        else // Motor already moving
        {
            // Update the math engine
            m_mathEngine.updateVelocityMove(sign, safeVel, safeAcc);
        }
    }

    void StepperMotor::emergencyStop(void)
    {
        m_controlTimer.stop();
        m_lastSentHz.reset();
        m_lastTickNsecs = 0;

        const uint8_t payload[] = {'1', m_byteId};
        sendPayload(payload, sizeof(payload));
    }

    void StepperMotor::enable(bool state)
    {
        throw "Not implemented";
    }

    void StepperMotor::home(void)
    {
        throw "Not implemented";
    }

    bool StepperMotor::isMoving(void) const
    {
        throw "Not implemented";
    }

    double StepperMotor::getEncoderPositionMm(void) const
    {
        // TODO: probably make the conversion from encoder to mm
        return m_positionGetter ? m_positionGetter() : 0.0;
    }

    void StepperMotor::onControlTick(void)
    {
        const int64_t currentNsecs = m_dtTimer.nsecsElapsed();
        const double dt            = static_cast<double>(currentNsecs - m_lastTickNsecs) / 1e9;

        m_lastTickNsecs = currentNsecs;

        const kinematic_state_t state = m_mathEngine.calculateNext(dt);

        if (state.isFinished) // Shutdown if complete
        {
            emergencyStop();
            return;
        }

        // Convert physical velocity to hardware frequency
        const uint16_t hz = computeFrequencyHz(std::abs(state.velocity), m_currentStepFraction);

        if (!m_lastSentHz.has_value() || m_lastSentHz.value() != hz)
        {
            const uint8_t direction  = (state.velocity >= 0.0) ? '1' : '0';
            const int32_t stepsCount = -1;
            // Send payload to the MCU (Fire and Forget)
            const uint8_t payload[] = {
                '2',
                m_byteId,
                direction,
                m_currentStepFraction,
                // Frequency
                static_cast<uint8_t>((hz >> 8) & 0xFF),
                static_cast<uint8_t>(hz & 0xFF),
                // Steps count
                static_cast<uint8_t>((stepsCount >> 24) & 0xFF),
                static_cast<uint8_t>((stepsCount >> 16) & 0xFF),
                static_cast<uint8_t>((stepsCount >> 8) & 0xFF),
                static_cast<uint8_t>(stepsCount & 0xFF),
            };

            sendPayload(payload, sizeof(payload));
            m_lastSentHz = hz;
        }
    }

    uint16_t StepperMotor::computeFrequencyHz(double velocityMmS, uint8_t stepFraction) const
    {
        const double stepsPerMm = (static_cast<double>(m_hwConfig.stepsPerRev) * stepFraction) / m_hwConfig.screwPitchMm;
        const double roundedHz  = std::round(velocityMmS * stepsPerMm);

        return static_cast<uint16_t>(std::clamp(roundedHz, 0.0, static_cast<double> UINT16_MAX));
    }

} // namespace Kub3::HAL::Act
