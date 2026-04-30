#pragma once

// Top level keys
#define CONF_KINEMATICS   "kinematics"
#define CONF_CAMERAS      "cameras"
#define CONF_FORCE_LIMITS "forceLimits"

// KINEMATICS inner keys
#define CONF_INITIAL_VELOCITY_MM_S "initial_velocity_mm_s"
#define CONF_TARGET_VELOCITY_MM_S  "target_velocity_mm_s"
#define CONF_ACCELERATION_MM_S     "acceleration_mm_s"
#define CONF_PARAMS_TYPE           "params_type"
#define CONF_PARAMS_TYPE_STEPPER   "stepper"
// --- Steppers
#define CONF_STEP_FRACTION "step_fraction"

// CAMERAS inner keys
#define CONF_MIN_CAMERA_DISTANCE_MM "min_camera_distance_mm"

// FORCE_LIMITS inner keys
#define CONF_HW_CRASH_FORCE_LIMIT_GF "hw_crash_force_limit_gf"
#define CONF_MAX_FORCE_GF            "max_force_gf"
#define CONF_CONTACT_THRESHOLD_GF    "contact_threshold_gf"
