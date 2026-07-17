#pragma once

#include <HAL/Actuators/IActuator.h>

namespace Kub3::HAL::Act
{

    class ISwitch : public IActuator
    {
    public:
        virtual ~ISwitch() = default;

        virtual void turnOn(void)  = 0;
        virtual void turnOff(void) = 0;

        [[nodiscard]] virtual bool isOn(void) const = 0;
    };

} // Kub3::HAL::Act
