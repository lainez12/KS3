#pragma once

#include <QElapsedTimer>
#include <QTimer>
#include <memory>
#include <string>
#include <vector>

#include "Config/machine_config.h"
#include "HAL/MCUDriver.h"
#include "IMotor.h"
#include "TrapezoidalGenerator.h"
#include "utils.h"

namespace Kub3::HAL::Act
{

    class StepperMotor final : public QObject, public IMotor
    {
        Q_OBJECT
    public:
        // MCUDriver is injected. We use a `weak_ptr` (if lifecycle is strictly guaranteed by `HardwareManager`)
        // `weak_ptr` prevents dangling references if MCU drops.
        StepperMotor(
            std::string id,
            uint8_t byteId,
            Weak<MCUDriver> driver,
            Config::stepper_hw_properties_t hwConfig,
            std::function<double()> positionGetter,
            QObject *parent = nullptr);

        [[nodiscard]] std::string_view getId(void) const noexcept override
        {
            return m_id;
        }

        void moveAbsolute(double position_mm, Config::kinematic_profile_t profile) override;
        void moveRelative(double distance_mm, Config::kinematic_profile_t profile) override;
        void moveDirection(MotorDirection dir, Config::kinematic_profile_t profile) override;
        void emergencyStop(void) override;
        void enable(bool state) override;
        void home(void) override;

        // Getters
        bool isMoving(void) const;
        [[nodiscard]] double getEncoderPositionMm(void) const override;

    private slots:
        void onControlTick(void);

    private:
        uint16_t computeFrequencyHz(double velocityMmS, uint8_t stepFraction) const;
        void sendPayload(const uint8_t *payload, uint32_t size) const;

    private:
        const std::string m_id;
        const uint8_t m_byteId;
        const Config::stepper_hw_properties_t m_hwConfig;
        Weak<MCUDriver> m_driver;
        std::function<double()> m_positionGetter;

        // Velocity curve and trajectory member variables
        uint8_t m_currentStepFraction = 1;
        TrapezoidalGenerator m_mathEngine;
        // --- Timers
        QTimer m_controlTimer;
        QElapsedTimer m_dtTimer;
        Optional<uint16_t> m_lastSentHz = std::nullopt;
        int64_t m_lastTickNsecs         = 0;
    };

} // namespace KUB3::HAL
