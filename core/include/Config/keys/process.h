#pragma once

// Top level keys
#define CONF_PROCESS_KINEMATICS          "kinematics"
#define CONF_PROCESS_CAMERAS             "cameras"
#define CONF_PROCESS_FORCE_LIMITS        "forceLimits"
#define CONF_PROCESS_ADMITTANCE_TUNING   "admittance"
#define CONF_PROCESS_ALIGNMENT_POSITIONS "alignmentPositions"
#define CONF_PROCESS_DRAWERS_POSITIONS   "drawersPositions"
#define CONF_PROCESS_ELEVATOR_POSITIONS  "zElevator"

// CONF_PROCESS_KINEMATICS inner keys
#define CONF_PROCESS_TARGET_VELOCITY_MM_S "target_velocity_mm_s"
#define CONF_PROCESS_ACCELERATION_MM_S    "acceleration_mm_s"
#define CONF_PROCESS_PARAMS_TYPE          "params_type"
#define CONF_PROCESS_PARAMS_TYPE_STEPPER  "stepper"
#define CONF_PROCESS_PARAMS_TYPE_GENERIC  "generic"
// --- Steppers
#define CONF_PROCESS_STEP_FRACTION "step_fraction"

// CONF_PROCESS_CAMERAS inner keys
#define CONF_PROCESS_MIN_CAMERA_DISTANCE_MM   "min_camera_distance_mm"
#define CONF_PROCESS_LEFT_CAM_X_RESET_POS_MM  "left_cam_x_reset_pos_mm"
#define CONF_PROCESS_LEFT_CAM_Y_RESET_POS_MM  "left_cam_y_reset_pos_mm"
#define CONF_PROCESS_RIGHT_CAM_X_RESET_POS_MM "right_cam_x_reset_pos_mm"
#define CONF_PROCESS_RIGHT_CAM_Y_RESET_POS_MM "right_cam_y_reset_pos_mm"
#define CONF_PROCESS_LEFT_CAM_X_HOME_POS_MM   "left_cam_x_home_pos_mm"
#define CONF_PROCESS_LEFT_CAM_Y_HOME_POS_MM   "left_cam_y_home_pos_mm"
#define CONF_PROCESS_RIGHT_CAM_X_HOME_POS_MM  "right_cam_x_home_pos_mm"
#define CONF_PROCESS_RIGHT_CAM_Y_HOME_POS_MM  "right_cam_y_home_pos_mm"
// Focals
#define CONF_PROCESS_LEFT_FOCAL          "leftFocal"  // --- Upper left camera focal
#define CONF_PROCESS_RIGHT_FOCAL         "rightFocal" // --- Upper right camera focal
#define CONF_PROCESS_FOCAL_DEFAULT_VALUE "focal_default_value"
#define CONF_PROCESS_FOCAL_MIN_VALUE     "focal_min_value"
#define CONF_PROCESS_FOCAL_MAX_VALUE     "focal_max_value"

// CONF_PROCESS_ELEVATOR_POSITIONS inner keys
#define CONF_PROCESS_MAX_Z_RELATIVE_DISTANCE_MM "max_z_relative_distance_mm"

// CONF_PROCESS_ALIGNMENT_POSITIONS inner keys
#define CONF_PROCESS_X_STAGE_CENTER_POS_MM     "x_stage_center_pos_mm"
#define CONF_PROCESS_Y_STAGE_CENTER_POS_MM     "y_stage_center_pos_mm"
#define CONF_PROCESS_THETA_STAGE_CENTER_POS_MM "theta_stage_center_pos_mm"

// CONF_PROCESS_DRAWERS_POSITIONS inner keys
#define CONF_PROCESS_CM3_RESET_POS_MM "cm3_reset_pos_mm"

// CONF_PROCESS_FORCE_LIMITS inner keys
#define CONF_PROCESS_HW_CRASH_FORCE_LIMIT_GF      "hw_crash_force_limit_gf"
#define CONF_PROCESS_MAX_FORCE_GF                 "max_force_gf"
#define CONF_PROCESS_CONTACT_THRESHOLD_GF         "contact_threshold_gf"
#define CONF_PROCESS_AUTOLEVEL_FORCE_GF           "autolevel_force_gf"
#define CONF_PROCESS_AUTOLEVEL_FORCE_TOLERANCE_GF "autolevel_force_tolerance_gf"

// CONF_PROCESS_ADMITTANCE_TUNING inner key
#define CONF_PROCESS_ADMITTANCE_MAX_STEP_MM_PER_TICK        "max_step_mm_per_tick"
#define CONF_PROCESS_ADMITTANCE_DEADBAND_VELOCITY_MM_S      "deadband_velocity_mm_s"
#define CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_LOW_FORCE  "translation_gain_low_force"
#define CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_HIGH_FORCE "translation_gain_high_force"
#define CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_LOW_FORCE     "rotation_gain_low_force"
#define CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_HIGH_FORCE    "rotation_gain_high_force"
