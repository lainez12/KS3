#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace Kub3::Config
{

    typedef struct stepper_kinematics_params_s
    {
        uint8_t stepFraction = 1; // Default value
    } stepper_kinematics_params_t;

    // Add necessary parameters to this variant for other motor types' kinematics
    using MotorKinematicsParams = std::variant<
        std::monostate,             // No specific params needed
        stepper_kinematics_params_t // Steppers
        >;

    typedef struct kinematic_profile_s
    {
        std::string id;
        double initialVelocityMmS = 0.0;
        double targetVelocityMmS;
        double accelerationMmS2;
        // Motor specific payload
        MotorKinematicsParams params = std::monostate{};
    } kinematic_profile_t;

}
