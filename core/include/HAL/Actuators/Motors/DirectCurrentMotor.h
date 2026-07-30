#pragma once

#include <QElapsedTimer>
#include <QTimer>
#include <string>
#include <string_view>

#include <Algorithms/Kinematic/IKinematicGenerator.h>
#include <Config/conf.h>
#include <HAL/MCUDriver.h>
#include <utils.h>

#include "IMotor.h"

namespace Kub3::HAL::Act
{

    using namespace Algorithms::Kinematic;

    class DirectCurrentMotor final : public QObject, public IMotor
    {
        Q_OBJECT
    public:
        DirectCurrentMotor(
            std::string id,
            uint8_t motorByteId,
            Weak<MCUDriver> driver,
            Config::dc_motor_hw_properties_t hwConfig,
            Unique<IKinematicGenerator> kinematicEngine,
            QObject *parent = nullptr);

        [[nodiscard]] std::string_view getId(void) const noexcept override
        {
            return m_id;
        }

        void moveDirection(MotorDirection dir, Config::kinematic_profile_t profile) override;
        void emergencyStop(void) override;

        // Getters
        [[nodiscard]] bool isMoving(void) const override;
        static std::function<void(const QByteArray &)> createFeedbackHandler(Shared<DirectCurrentMotor> motor);

    private slots:
        void onControlTick(void);

    private:
        void sendTorqueLimits() const;
        uint16_t computePwm(double velocityMmS) const;
        void sendPayload(const QByteArray &payload) const;
        void resetInternalState(void);

    private:
        const std::string m_id;
        const uint8_t m_motorByteId;
        const Config::dc_motor_hw_properties_t m_hwConfig;
        Weak<MCUDriver> m_driver;
        Unique<IKinematicGenerator> m_kinematicEngine;

        // --- Timers & Caches
        QTimer m_controlTimer;
        QElapsedTimer m_dtTimer;

        // Cache to prevent spamming the Serial port if speed/direction hasn't changed
        Optional<uint16_t> m_lastSentPwm = std::nullopt;
        Optional<uint8_t> m_lastSentDir  = std::nullopt;

        int64_t m_lastTickNsecs = 0;
    };

    using DCMotor = DirectCurrentMotor;

} // namespace Kub3::HAL::Act
