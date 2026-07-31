#pragma once

#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <utils.h>

namespace Kub3::Services
{

    typedef struct stowage_mask_motor_bundle_s {
        Shared<HAL::Act::IPositionMotor> motor;
        Config::kinematic_profile_t kinematics;
    } stowage_mask_motor_bundle_t;

}