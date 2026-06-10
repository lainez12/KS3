#pragma once

#include <stdint.h>

#include "../IActuator.h"

namespace Kub3::HAL::Act
{

    class IFocal : public IActuator
    {
    public:
        virtual ~IFocal() = default;

        virtual void enable()               = 0;
        virtual void disable()              = 0;
        virtual void setValue(uint16_t val) = 0;

        [[nodiscard]] virtual bool isEnabled(void) const    = 0;
        [[nodiscard]] virtual uint16_t getValue(void) const = 0;
    };

}
