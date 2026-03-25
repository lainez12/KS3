#pragma once

#include <cstdint>

#include "IActuator.h"

#define POSITIVE_INFINITE INT32_MAX
#define NEGATIVE_INFINITE INT32_MAX

namespace Kub3::HAL::Act
{

    class IMotor : public virtual IActuator
    {
    public:
        virtual void moveAbsolute(int32_t position_mm)      = 0;
        virtual void moveRelative(int32_t distance_mm)      = 0;
        virtual void setTargetSpeed(uint32_t velocity_mm_s) = 0;
        virtual void home(void)                             = 0;

        virtual bool isMoving(void) const           = 0;
        virtual uint32_t getTargetSpeed(void) const = 0;
    };

} // namespace KUB3::HAL
