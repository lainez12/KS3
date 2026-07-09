#pragma once

#include <QMetaType>
#include <cstdint>
#include <string>
#include <variant>

#include <Config/default/process.h>

namespace Kub3::Config
{

    typedef struct stepper_kinematics_params_s {
        uint8_t stepFraction = CONF_PROCESS_STEP_FRACTION_DEFAULT; // Default value
    } stepper_kinematics_params_t;

    // Add necessary parameters to this variant for other motor types' kinematics
    using MotorKinematicsParams = std::variant<
        std::monostate,             // No specific params needed
        stepper_kinematics_params_t // Steppers
        >;

    typedef struct kinematic_profile_s {
        std::string id;
        double targetVelocityMmS = CONF_PROCESS_TARGET_VELOCITY_MM_S_DEFAULT;
        double accelerationMmS2  = CONF_PROCESS_ACCELERATION_MM_S_DEFAULT;
        // Motor specific payload
        MotorKinematicsParams params = std::monostate{};
    } kinematic_profile_t;

}

Q_DECLARE_METATYPE(Kub3::Config::kinematic_profile_t)
