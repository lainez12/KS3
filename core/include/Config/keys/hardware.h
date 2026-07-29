#pragma once

// Top level keys
#define CONF_HW_MCUS          "mcus" // Micro-controller units
#define CONF_HW_MOTORS        "motors"
#define CONF_HW_CAMERAS       "cameras"
#define CONF_HW_FORCE_SENSORS "forceSensors"

// CONF_HW_MCUS inner keys
#define CONF_HW_MCU1_PORT     "mcu1_port"
#define CONF_HW_MCU2_PORT     "mcu2_port"
#define CONF_HW_MCU3_PORT     "mcu3_port"
#define CONF_HW_MCU4_PORT     "mcu4_port"
#define CONF_HW_MCU1_BAUDRATE "mcu1_baudrate"
#define CONF_HW_MCU2_BAUDRATE "mcu2_baudrate"
#define CONF_HW_MCU3_BAUDRATE "mcu3_baudrate"
#define CONF_HW_MCU4_BAUDRATE "mcu4_baudrate"

// CONF_HW_MOTORS inner keys
#define CONF_HW_MOTOR_TYPE         "type"
#define CONF_HW_MOTOR_TYPE_STEPPER "stepper"
#define CONF_HW_MOTOR_TYPE_DC      "dc"
// --- Common
#define CONF_HW_SCREW_PITCH_MM         "screw_pitch_mm"
#define CONF_HW_MAX_VELOCITY_MM_S      "max_velocity_mm_s"
#define CONF_HW_MAX_ACCELERATION_MM_S2 "max_acceleration_mm_s2"
// --- With encoder
#define CONF_HW_ENCODER_TOPS_PER_REV "encoder_tops_per_rev"
// --- Steppers
#define CONF_HW_MOTOR_STEPS_PER_REV "steps_per_rev"
// --- Direct Current
#define CONF_HW_MOTOR_MAX_POSITIVE_TORQUE "max_positive_torque"
#define CONF_HW_MOTOR_MAX_NEGATIVE_TORQUE "max_negative_torque"

// CONF_HW_CAMERAS inner keys
#define CONF_HW_SERIAL_NUMBER       "serial_number"
#define CONF_HW_MAX_EXPOSURE_US     "max_exposure_us"
#define CONF_HW_DEFAULT_EXPOSURE_US "default_exposure_us"
#define CONF_HW_MAX_GAIN_DB         "max_gain_db"
#define CONF_HW_DEFAULT_GAIN_DB     "default_gain_db"
#define CONF_HW_FRAMERATE           "default_framerate"
#define CONF_HW_ASSOCIATED_FOCAL_ID "associated_focal_id"
#define CONF_HW_ASSOCIATED_LIGHT_ID "associated_light_id"

// CONF_HW_FORCE_SENSORS inner keys
#define CONF_HW_ADC_TO_GRAM_FORCE_FACTOR "adc_to_gram_force_factor"

// KEYBOARD LAYOUTS (IETF language tag)
#define CONF_HW_KEYBOARD_LAYOUT "keyboard_layout"
#define LOCALE_EN_US            "en_US"
#define LOCALE_FR_FR            "fr_FR"
#define LOCALE_EN_GB            "en_GB"
#define LOCALE_ES_ES            "es_ES"
#define LOCALE_DE_DE            "de_DE"
