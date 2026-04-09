#pragma once

#include <cstdint>
#include <unordered_map>
#include <variant>

#include "../IActuator.h"
#include "Config/kinematics.h"

#define POSITIVE_INFINITE INT32_MAX
#define NEGATIVE_INFINITE INT32_MAX

namespace Kub3::HAL::Act
{

    enum class MotorDirection
    {
        Positive, // Away from the motor
        Negative  // Towards the motor
    };

    class IMotor : public virtual IActuator
    {
    public:
        virtual void moveAbsolute(double position_mm, Config::kinematic_profile_t profile)  = 0;
        virtual void moveRelative(double distance_mm, Config::kinematic_profile_t profile)  = 0;
        virtual void moveDirection(MotorDirection dir, Config::kinematic_profile_t profile) = 0;
        virtual void home(void)                                                             = 0;

        virtual bool isMoving(void) const                             = 0;
        [[nodiscard]] virtual double getEncoderPositionMm(void) const = 0;
    };

} // namespace KUB3::HAL
