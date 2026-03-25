#pragma once

#include <memory>
#include <string>
#include <vector>

#include "HAL/MCUDriver.h"
#include "IMotor.h"

namespace Kub3::HAL::Act
{

    class StepperMotor final : public IMotor
    {
    public:
        // MCUDriver is injected. We use a `weak_ptr` (if lifecycle is strictly guaranteed by `HardwareManager`)
        // `weak_ptr` prevents dangling references if MCU drops.
        StepperMotor(std::string id, Weak<MCUDriver> driver);

        [[nodiscard]] std::string_view getId() const noexcept override
        {
            return m_id;
        }

        void moveAbsolute(int32_t position_mm) override;
        void moveRelative(int32_t distance_mm) override;
        void setTargetSpeed(uint32_t velocity_mm_s) override;
        void emergencyStop() override;
        void enable(bool state) override;
        void home() override;

        // Getters
        bool isMoving(void) const;
        uint32_t getTargetSpeed(void) const override;

    private:
        void sendPayload(uint8_t *payload, uint32_t size) const;

    private:
        const std::string m_id;
        bool m_moving          = false;
        uint32_t m_targetSpeed = 0;

        Weak<MCUDriver> m_driver;
    };

} // namespace KUB3::HAL
