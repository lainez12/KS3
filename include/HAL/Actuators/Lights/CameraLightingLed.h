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
        CameraLightingLed(std::string id, uint8_t byteId, Weak<MCUDriver> driver);

        void enable(void) override;
        void disable(void) override;
        void setValue(uint16_t val) override;
        void emergencyStop(void) override;

        [[nodiscard]] std::string_view getId(void) const noexcept override
        {
            return m_id;
        }
        [[nodiscard]] bool isEnabled(void) const override
        {
            return m_enabled;
        }
        [[nodiscard]] uint16_t getValue(void) const override
        {
            return m_value;
        }

    private:
        void sendCommand(void);

    private:
        const std::string m_id;
        const uint8_t m_byteId;
        Weak<MCUDriver> m_driver;
        // State vars
        std::atomic<bool> m_enabled   = false;
        std::atomic<uint16_t> m_value = 0;
    };

}
