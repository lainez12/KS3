#pragma once

// Top level keys
#define CONF_MOTORS        "motors"
#define CONF_CAMERAS       "cameras"
#define CONF_FORCE_SENSORS "forceSensors"

// CONF_MOTORS inner keys
#define CONF_MOTOR_TYPE         "type"
#define CONF_MOTOR_TYPE_STEPPER "stepper"
#define CONF_MOTOR_TYPE_DC      "dc"
// --- Common
#define CONF_SCREW_PITCH_MM         "screw_pitch_mm"
#define CONF_MAX_VELOCITY_MM_S      "max_velocity_mm_s"
#define CONF_MAX_ACCELERATION_MM_S2 "max_acceleration_mm_s2"
#define CONF_ENCODER_TOPS_PER_REV   "encoder_tops_per_rev"
// --- Steppers
#define CONF_MOTOR_STEPS_PER_REV "steps_per_rev"

// CONF_CAMERAS inner keys
#define CONF_SERIAL_NUMBER       "serial_number"
#define CONF_MAX_EXPOSURE_US     "max_exposure_us"
#define CONF_DEFAULT_EXPOSURE_US "default_exposure_us"
#define CONF_MAX_GAIN_DB         "max_gain_db"
#define CONF_DEFAULT_GAIN_DB     "default_gain_db"

// CONF_FORCE_SENSORS inner keys
#define ADC_TO_GRAM_FORCE_FACTOR "adc_to_gram_force_factor"
