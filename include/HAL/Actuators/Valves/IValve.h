#pragma once

#include "../IActuator.h"

namespace Kub3::HAL::Act
{

    class IValve : public IActuator
    {
    public:
        virtual ~IValve() = default;

        virtual void open(void)  = 0;
        virtual void close(void) = 0;

        [[nodiscard]] virtual bool isOpen(void) const = 0;
    };

} // Kub3::HAL::Act
