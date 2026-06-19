#pragma once

#include <stdint.h>

#include <HAL/Actuators/IActuator.h>

namespace Kub3::HAL::Act
{

    class ILightingSystem : public IActuator
    {
    public:
        virtual ~ILightingSystem() = default;

        virtual void enable()                     = 0;
        virtual void disable()                    = 0;
        virtual void setValueFraction(double val) = 0;

        [[nodiscard]] virtual bool isEnabled(void) const          = 0;
        [[nodiscard]] virtual double getValueFraction(void) const = 0;
    };

}
