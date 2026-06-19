#pragma once

#include <atomic>

#include <HAL/MCUDriver.h>
#include <utils.h>

#include "ILightingSystem.h"

namespace Kub3::HAL::Act
{

    class CameraLightingLed final : public ILightingSystem
    {
    public:
        CameraLightingLed(std::string id, uint8_t byteId, uint16_t maximumValue, Weak<MCUDriver> driver);

        void enable(void) override;
        void disable(void) override;
        void setValueFraction(double val) override;
        void emergencyStop(void) override;

        [[nodiscard]] std::string_view getId(void) const noexcept override
        {
            return m_id;
        }
        [[nodiscard]] bool isEnabled(void) const override
        {
            return m_enabled;
        }
        [[nodiscard]] double getValueFraction(void) const override
        {
            return m_valueFraction;
        }

    private:
        void sendCommand(void);

    private:
        const std::string m_id;
        const uint8_t m_byteId;
        const uint16_t m_maximumValue;
        Weak<MCUDriver> m_driver;
        // State vars
        std::atomic<bool> m_enabled         = false;
        std::atomic<double> m_valueFraction = 0.0; // Percentage of maximum value
    };

}
