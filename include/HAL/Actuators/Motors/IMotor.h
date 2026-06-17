#pragma once

#include <cstdint>
#include <unordered_map>
#include <variant>

#include <Config/kinematics.h>

#include <HAL/Actuators/IActuator.h>

#define POSITIVE_INFINITE INT32_MAX
#define NEGATIVE_INFINITE INT32_MAX

namespace Kub3::HAL::Act
{

    enum class MotorDirection
    {
        Positive = 0x0, // Away from the motor
        Negative = 0x1  // Towards the motor
    };

    class IMotor : public IActuator
    {
    public:
        virtual ~IMotor() = default;

        virtual void moveDirection(MotorDirection dir, Config::kinematic_profile_t profile) = 0;
        virtual bool isMoving(void) const                                                   = 0;
    };

} // namespace KUB3::HAL
