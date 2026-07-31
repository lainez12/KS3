#include <QDebug>
#include <QMetaObject>
#include <algorithm>
#include <qstringview.h>
#include <stdexcept>

#include <HAL/Actuators/Motors/DirectCurrentMotor.h>

#define CONTROL_TIMER_VALUE_MS 20 // 50Hz

namespace Kub3::HAL::Act
{

    DirectCurrentMotor::DirectCurrentMotor(std::string id,
                                           uint8_t motorByteId,
                                           Weak<MCUDriver> driver,
                                           Config::dc_motor_hw_properties_t hwConfig,
                                           Unique<IKinematicGenerator> kinematicEngine,
                                           QObject *parent) :
        QObject(parent),
        m_id(std::move(id)),
        m_motorByteId(motorByteId),
        m_hwConfig(std::move(hwConfig)),
        m_driver(std::move(driver)),
        m_kinematicEngine(std::move(kinematicEngine)),
        m_controlTimer(this)
    {
        qInfo() << std::format("DirectCurrentMotor[{}] Loaded config:", m_id).c_str();
        qInfo() << std::format("--- maxVelocityMmS={}", m_hwConfig.maxVelocityMmS).c_str();
        qInfo() << std::format("--- maxAccelerationMmS2={}", m_hwConfig.maxAccelerationMmS2).c_str();

        if (!m_kinematicEngine)
        {
            throw std::runtime_error(std::format("DirectCurrentMotor[{}] was provided a null kinematic engine.", m_id));
        }

        connect(&m_controlTimer, &QTimer::timeout, this, &DirectCurrentMotor::onControlTick);
    }

    void DirectCurrentMotor::sendPayload(const QByteArray &payload) const
    {
        if (auto driver = m_driver.lock())
        {
            driver->sendCommand(payload);
        }
        else
        {
            throw std::runtime_error("Attempted to send command, but MCUDriver is dead. Actuator: " + m_id);
        }
    }

    // TODO: profile could be a const reference ?
    void DirectCurrentMotor::moveDirection(MotorDirection dir, Config::kinematic_profile_t profile)
    {
        QMetaObject::invokeMethod(
            this,
            [this, dir, profile]() {
                const double safeVel = std::min(profile.targetVelocityMmS, m_hwConfig.maxVelocityMmS);
                const double safeAcc = std::min(profile.accelerationMmS2, m_hwConfig.maxAccelerationMmS2);
                const double sign    = (dir == MotorDirection::Positive) ? 1.0 : -1.0;

                m_lastSentPwm.reset();
                m_lastSentDir.reset();

                this->sendTorqueLimits(); // Send/Resend torque limit before moving
                if (!m_controlTimer.isActive())
                {
                    m_kinematicEngine->startVelocityMove(0.0, sign, safeVel, safeAcc);

                    m_lastTickNsecs = 0;
                    m_dtTimer.start();
                    m_controlTimer.start(CONTROL_TIMER_VALUE_MS);
                }
                else
                {
                    m_kinematicEngine->updateVelocityMove(sign, safeVel, safeAcc);
                }
            });
    }

    void DirectCurrentMotor::emergencyStop(void)
    {
        QMetaObject::invokeMethod(
            this,
            [this]() {
                m_controlTimer.stop();
                m_lastSentPwm.reset();
                m_lastSentDir.reset();
                m_lastTickNsecs = 0;

                // Command format: C<MOTOR>S
                const char tmpCmd[] = {'C', static_cast<char>(m_motorByteId), 'S'};
                QByteArray cmd(tmpCmd, sizeof(tmpCmd));
                sendPayload(cmd);
            },
            Qt::AutoConnection);
    }

    bool DirectCurrentMotor::isMoving(void) const
    {
        return m_controlTimer.isActive();
    }

    void DirectCurrentMotor::onControlTick(void)
    {
        const int64_t currentNsecs = m_dtTimer.nsecsElapsed();
        const double dt            = static_cast<double>(currentNsecs - m_lastTickNsecs) / 1e9;

        m_lastTickNsecs = currentNsecs;

        const kinematic_state_t state = m_kinematicEngine->computeNext(dt);

        if (state.isFinished)
        {
            emergencyStop();
            return;
        }

        // Determine protocol direction based on velocity sign
        const uint8_t dir  = (state.velocity >= 0.0) ? 'F' : 'B';
        const uint16_t pwm = computePwm(std::abs(state.velocity));

        // Fire and Forget only if PWM or Direction changed
        if (!m_lastSentPwm.has_value() || m_lastSentPwm.value() != pwm ||
            !m_lastSentDir.has_value() || m_lastSentDir.value() != dir)
        {
            // Command format: C<MOTOR><SENS><VITESSE>
            const QByteArray pwmBytes = QByteArray::number(pwm); // pwm stringified (ASCII digits)
            QByteArray command;

            command.reserve(3 + pwmBytes.size());
            // "C" + m_motorByteId + dir + pwm(as ASCII bytes)
            command.append(static_cast<char>('C'));
            command.append(static_cast<char>(m_motorByteId));
            command.append(static_cast<char>(dir));
            command.append(pwmBytes);

            sendPayload(command);

            m_lastSentPwm = pwm;
            m_lastSentDir = dir;
        }
    }

    void DirectCurrentMotor::sendTorqueLimits() const
    {
        auto _sendSingleTorqueLimit = [this](const uint16_t limit, const char dirByte) {
            // Command format: <MOTOR><SENS><TORQUE LIMIT>
            const QByteArray torqueLimitBytes = QByteArray::number(limit);
            QByteArray cmd;

            cmd.reserve(2 + torqueLimitBytes.size());
            cmd.append(static_cast<char>(m_motorByteId));
            cmd.append(dirByte);
            cmd.append(torqueLimitBytes);

            this->sendPayload(cmd);
        };

        _sendSingleTorqueLimit(m_hwConfig.maxNegativeTorque, 'B'); // 'B' for backward (negative movement)
        _sendSingleTorqueLimit(m_hwConfig.maxPositiveTorque, 'F'); // 'F' for forward (positive movement)
    }

    uint16_t DirectCurrentMotor::computePwm(double velocityMmS) const
    {
        // Safety guard against division by zero
        if (m_hwConfig.maxVelocityMmS <= 0.0)
            return 0;

        // Map absolute velocity (0 to MaxVel) to PWM (0 to 4095)
        const double ratio  = velocityMmS / m_hwConfig.maxVelocityMmS;
        const double rawPwm = std::round(ratio * 4095.0);

        return static_cast<uint16_t>(std::clamp(rawPwm, 0.0, 4095.0));
    }

} // namespace Kub3::HAL::Act
