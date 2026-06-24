#include <QDebug>
#include <QMetaObject>

#include <algorithm>
#include <stdexcept>

#include <Config/helper.h>
#include <HAL/Actuators/Motors/StepperMotor.h>

#define CONTROL_TIMER_VALUE_MS 20 // 50Hz

namespace Kub3::HAL::Act
{

    StepperMotor::StepperMotor(std::string id,
                               uint8_t byteId,
                               Weak<MCUDriver> driver,
                               Config::stepper_hw_properties_t hwConfig,
                               std::function<int32_t()> posGetter,
                               std::string encoderId,
                               Unique<IKinematicGenerator> kinematicEngine,
                               QObject *parent) :
        QObject(parent),
        m_id(std::move(id)),
        m_byteId(byteId),
        m_hwConfig(std::move(hwConfig)),
        m_driver(std::move(driver)),
        m_encoderValueGetter(std::move(posGetter)),
        m_encoderId(std::move(encoderId)),
        m_kinematicEngine(std::move(kinematicEngine)),
        m_controlTimer(this)
    {
        if (!m_kinematicEngine)
        {
            throw std::format("Stepper[{}] was provided a null kinematic engine.", m_id);
        }

        connect(&m_controlTimer, &QTimer::timeout, this, &StepperMotor::onControlTick);
    }

    void StepperMotor::moveAbsolute(double position_mm, Config::kinematic_profile_t profile)
    {
        QMetaObject::invokeMethod(
            this,
            [this, position_mm, profile]() {
                // Clamp to safety limits
                const double safeVel     = std::min(profile.targetVelocityMmS, m_hwConfig.maxVelocityMmS);
                const double safeAcc     = std::min(profile.accelerationMmS2, m_hwConfig.maxAccelerationMmS2);
                const double precisionMm = this->getPrecisionMm(profile);

                // Extract step fraction safely
                uint8_t stepFrac = 1;
                if (auto *p = std::get_if<Config::stepper_kinematics_params_t>(&profile.params))
                    stepFrac = p->stepFraction;

                m_currentStepFraction = stepFrac; // Store for MCU translation
                m_lastSentHz.reset();             // Reset cached "last sent frequency" value;

                if (!m_controlTimer.isActive()) // Start the control timer
                {
                    // Initialize the math engine
                    m_kinematicEngine->startPositionMove(getEncoderPositionMm(), position_mm, safeVel, safeAcc, precisionMm);

                    m_lastTickNsecs = 0; // Reset last tick timestamp
                    m_dtTimer.start();   // reset elapsed timer
                    m_controlTimer.start(CONTROL_TIMER_VALUE_MS);
                }
                else // Motor already moving
                {
                    // Update the math engine
                    m_kinematicEngine->updatePositionMove(position_mm, safeVel, safeAcc, precisionMm);
                }
            },
            Qt::QueuedConnection);
    }

    void StepperMotor::moveRelative(double distance_mm, Config::kinematic_profile_t profile)
    {
        QMetaObject::invokeMethod(
            this,
            [this, distance_mm, profile]() {
                // Clamp to safety limits
                const double safeVel     = std::min(profile.targetVelocityMmS, m_hwConfig.maxVelocityMmS);
                const double safeAcc     = std::min(profile.accelerationMmS2, m_hwConfig.maxAccelerationMmS2);
                const double precisionMm = this->getPrecisionMm(profile);

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
                    m_kinematicEngine->startPositionMove(encoderPos, encoderPos + distance_mm, safeVel, safeAcc, precisionMm);

                    m_lastTickNsecs = 0; // Reset last tick timestamp
                    m_dtTimer.start();   // reset elapsed timer
                    m_controlTimer.start(CONTROL_TIMER_VALUE_MS);
                }
                else // Motor already moving
                {
                    const double currentMathEnginePos = m_kinematicEngine->getCurrentState().position;

                    // Update the math engine
                    m_kinematicEngine->updatePositionMove(currentMathEnginePos + distance_mm, safeVel, safeAcc, precisionMm);
                }
            },
            Qt::QueuedConnection);
    }

    void StepperMotor::moveDirection(MotorDirection dir, Config::kinematic_profile_t profile)
    {
        // qDebug().noquote() << QString("[StepperMotor::moveDirection] Dir %1 with kinematics:\n%2")
        //                           .arg(static_cast<int>(dir) ? "Negative" : "Positive")
        //                           .arg(Config::toString(profile));
        QMetaObject::invokeMethod(
            this,
            [this, dir, profile]() {
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
                    m_kinematicEngine->startVelocityMove(getEncoderPositionMm(), sign, safeVel, safeAcc);

                    m_lastTickNsecs = 0; // Reset last tick timestamp
                    m_dtTimer.start();   // reset elapsed timer
                    m_controlTimer.start(CONTROL_TIMER_VALUE_MS);
                }
                else // Motor already moving
                {
                    // Update the math engine
                    m_kinematicEngine->updateVelocityMove(sign, safeVel, safeAcc);
                }
            },
            Qt::QueuedConnection);
    }

    void StepperMotor::emergencyStop(void)
    {
        QMetaObject::invokeMethod(
            this,
            [this]() {
                this->resetInternalState();

                const uint8_t payload[] = {'1', m_byteId};
                sendPayload(payload, sizeof(payload));
            },
            Qt::QueuedConnection);
    }

    void StepperMotor::resetEncoder(const double offsetMm)
    {
        QMetaObject::invokeMethod(
            this,
            [this, offsetMm]() {
                const double topsPerMm    = m_hwConfig.encoderTopsPerRev / m_hwConfig.screwPitchMm;
                const int32_t encoderTops = std::round(offsetMm * topsPerMm);
                const uint8_t payload[]   = {
                    'R',
                    m_byteId,
                    static_cast<uint8_t>((encoderTops >> 24) & 0xFF),
                    static_cast<uint8_t>((encoderTops >> 16) & 0xFF),
                    static_cast<uint8_t>((encoderTops >> 8) & 0xFF),
                    static_cast<uint8_t>(encoderTops & 0xFF)};

                sendPayload(payload, sizeof(payload));
            },
            Qt::QueuedConnection);
    }

    bool StepperMotor::isMoving(void) const
    {
        return m_controlTimer.isActive();
    }

    double StepperMotor::getEncoderPositionMm(void) const
    {
        const int32_t encoderValue = m_encoderValueGetter ? m_encoderValueGetter() : 0;

        return static_cast<double>(encoderValue) * (m_hwConfig.screwPitchMm / static_cast<double>(m_hwConfig.encoderTopsPerRev));
    }

    std::function<void(const QByteArray &)> StepperMotor::createFeedbackHandler(Shared<StepperMotor> motor)
    {
        // TODO:
        // - Currently no way to know the message kind using the current communication protocol
        // - Thankfully for now only one exists: motor stopped

        return [weakMotor = Weak<StepperMotor>(motor)](const QByteArray &payload) {
            if (auto safeMotor = weakMotor.lock())
            {
                if (!payload.isEmpty())
                {
#if defined(BUILD_DEBUG)
                    qDebug().nospace() << "[StepperMotor](" << safeMotor->m_id << ") Received feedback: Stopped with code " << payload.toHex(' ');
#endif
                    QMetaObject::invokeMethod(safeMotor.get(), [m = safeMotor]() { m->resetInternalState(); }, Qt::AutoConnection);
                }
            }
        };
    }

    void StepperMotor::onControlTick(void)
    {
        const int64_t currentNsecs = m_dtTimer.nsecsElapsed();
        const double dt            = static_cast<double>(currentNsecs - m_lastTickNsecs) / 1e9;

        m_lastTickNsecs = currentNsecs;

        const kinematic_state_t state = m_kinematicEngine->computeNext(dt, getEncoderPositionMm());

#if defined(BUILD_DEBUG)
        qDebug() << "===============================";
        qDebug() << "Current position (mm, encoder):" << getEncoderPositionMm();
        qDebug() << "Computed next position (mm, generator state):" << state.position;
        qDebug() << "Computed velocity (mm/s, generator state):" << state.velocity;
        qDebug() << "Is finished (generator state):" << state.isFinished;
#endif

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

    double StepperMotor::getPrecisionMm(const Config::kinematic_profile_t &profile)
    {
        uint8_t stepFrac = 1; // Default to full step

        if (auto *p = std::get_if<Config::stepper_kinematics_params_t>(&profile.params))
            stepFrac = p->stepFraction;

        // The true positional precision of the system is bottlenecked by the coarser
        // resolution between what the motor can command and what the encoder can measure.
        // - If the encoder has a lower resolution than the microstepped motor, we cannot verify tiny movements.
        // - If the motor operates in coarse steps, we cannot physically position it at the fine increments the encoder can see.
        // By taking the minimum of the two (std::min), we identify the system's limiting resolution per revolution.
        // Dividing the lead screw pitch by this bottleneck gives the smallest reliable linear movement in mm.
        const uint32_t steps_count = m_hwConfig.stepsPerRev * stepFrac;
        const uint32_t divisor     = std::min<uint32_t>(steps_count, m_hwConfig.encoderTopsPerRev);

        qInfo() << "[StepperMotor] Computed precision (mm):" << (m_hwConfig.screwPitchMm / divisor);
        return (m_hwConfig.screwPitchMm / divisor);
    }

    uint16_t StepperMotor::computeFrequencyHz(double velocityMmS, uint8_t stepFraction) const
    {
        const double stepsPerMm = (static_cast<double>(m_hwConfig.stepsPerRev) * stepFraction) / m_hwConfig.screwPitchMm;
        const double roundedHz  = std::round(velocityMmS * stepsPerMm);

        return static_cast<uint16_t>(std::clamp(roundedHz, 0.0, static_cast<double>(UINT16_MAX)));
    }

    void StepperMotor::sendPayload(const uint8_t *data, uint32_t size) const
    {
        QByteArray payload = QByteArray(reinterpret_cast<const char *>(data), size);

        if (auto driver = m_driver.lock())
        {
            driver->sendCommand(payload);
        }
        else
        {
            throw std::runtime_error("Attempted to send command, but MCUDriver is dead. Actuator: " + m_id);
        }
    }

    void StepperMotor::resetInternalState(void)
    {
        m_controlTimer.stop();
        m_lastSentHz.reset();
        m_lastTickNsecs = 0;
    }

} // namespace Kub3::HAL::Act
