#pragma once

#include "IActuator.h"

namespace Kub3::HAL::Act
{

    class IMotor : public virtual IActuator
    {
    public:
        virtual void moveAbsolute(double position_mm)  = 0;
        virtual void moveRelative(double distance_mm)  = 0;
        virtual void setVelocity(double velocity_mm_s) = 0;
        virtual void home()                            = 0;
    };

} // namespace KUB3::HAL
