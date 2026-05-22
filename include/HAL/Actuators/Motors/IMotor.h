#pragma once

#include <cstdint>
#include <unordered_map>
#include <variant>

#include <Config/kinematics.h>

#include "../IActuator.h"

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

        virtual void moveAbsolute(double position_mm, Config::kinematic_profile_t profile)  = 0;
        virtual void moveRelative(double distance_mm, Config::kinematic_profile_t profile)  = 0;
        virtual void moveDirection(MotorDirection dir, Config::kinematic_profile_t profile) = 0;
        virtual void resetEncoder(const double offsetMm = 0.0)                              = 0;

        virtual bool isMoving(void) const                               = 0;
        [[nodiscard]] virtual std::string_view getEncoderId(void) const = 0;
        [[nodiscard]] virtual double getEncoderPositionMm(void) const   = 0;

    private:
        virtual double computePrecisionMm(const Config::kinematic_profile_t &profile) = 0;
    };

} // namespace KUB3::HAL
