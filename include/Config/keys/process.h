#pragma once

// Top level keys
#define CONF_PROCESS_KINEMATICS          "kinematics"
#define CONF_PROCESS_CAMERAS             "cameras"
#define CONF_PROCESS_FORCE_LIMITS        "forceLimits"
#define CONF_PROCESS_ALIGNMENT_POSITIONS "alignmentPositions"
#define CONF_PROCESS_DRAWERS_POSITIONS   "drawersPositions"

// CONF_PROCESS_KINEMATICS inner keys
#define CONF_PROCESS_INITIAL_VELOCITY_MM_S "initial_velocity_mm_s"
#define CONF_PROCESS_TARGET_VELOCITY_MM_S  "target_velocity_mm_s"
#define CONF_PROCESS_ACCELERATION_MM_S     "acceleration_mm_s"
#define CONF_PROCESS_PARAMS_TYPE           "params_type"
#define CONF_PROCESS_PARAMS_TYPE_STEPPER   "stepper"
#define CONF_PROCESS_PARAMS_TYPE_GENERIC   "generic"
// --- Steppers
#define CONF_PROCESS_STEP_FRACTION "step_fraction"

// CONF_PROCESS_CAMERAS inner keys
#define CONF_PROCESS_MIN_CAMERA_DISTANCE_MM   "min_camera_distance_mm"
#define CONF_PROCESS_LEFT_CAM_X_RESET_POS_MM  "left_cam_x_reset_pos_mm"
#define CONF_PROCESS_LEFT_CAM_Y_RESET_POS_MM  "left_cam_y_reset_pos_mm"
#define CONF_PROCESS_RIGHT_CAM_X_RESET_POS_MM "right_cam_x_reset_pos_mm"
#define CONF_PROCESS_RIGHT_CAM_Y_RESET_POS_MM "right_cam_y_reset_pos_mm"

// CONF_PROCESS_ALIGNMENT_POSITIONS inner keys
#define CONF_PROCESS_X_STAGE_CENTER_POS_MM     "x_stage_center_pos_mm"
#define CONF_PROCESS_Y_STAGE_CENTER_POS_MM     "y_stage_center_pos_mm"
#define CONF_PROCESS_THETA_STAGE_CENTER_POS_MM "theta_stage_center_pos_mm"

// CONF_PROCESS_DRAWERS_POSITIONS inner keys
#define CONF_PROCESS_CM3_RESET_POS_MM "cm3_reset_pos_mm"

// CONF_PROCESS_FORCE_LIMITS inner keys
#define CONF_PROCESS_HW_CRASH_FORCE_LIMIT_GF "hw_crash_force_limit_gf"
#define CONF_PROCESS_MAX_FORCE_GF            "max_force_gf"
#define CONF_PROCESS_CONTACT_THRESHOLD_GF    "contact_threshold_gf"
