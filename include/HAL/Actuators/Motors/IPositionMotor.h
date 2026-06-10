#pragma once

#include <cstdint>
#include <unordered_map>
#include <variant>

#include <Config/kinematics.h>

#include <HAL/Actuators/Motors/IMotor.h>

namespace Kub3::HAL::Act
{
    class IPositionMotor : public IMotor
    {
    public:
        virtual ~IPositionMotor() = default;

        virtual void moveAbsolute(double position_mm, Config::kinematic_profile_t profile) = 0;
        virtual void moveRelative(double distance_mm, Config::kinematic_profile_t profile) = 0;
        virtual void resetEncoder(const double offsetMm = 0.0)                             = 0;

        [[nodiscard]] virtual std::string_view getEncoderId(void) const = 0;
        [[nodiscard]] virtual double getEncoderPositionMm(void) const   = 0;

    private:
        virtual double computePrecisionMm(const Config::kinematic_profile_t &profile) = 0;
    };

} // namespace KUB3::HAL
