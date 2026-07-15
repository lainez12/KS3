#pragma once

#include <QString>
#include <format>
#include <string>
#include <unordered_map>
#include <variant>

#include "../utils.h" // @note: keep relative path for compilation
#include <Config/default/hardware.h>

#define MCU_COUNT            4
#define DEFAULT_MCU_BAUDRATE 115200

namespace Kub3::Config
{

    ///////////////////////////
    // HARDWARE CONFIGURATION
    ///////////////////////////

    typedef struct mcu_hw_properties_s {
        QString port;
        uint32_t baudrate = DEFAULT_MCU_BAUDRATE;
    } mcu_hw_properties_t;

    typedef struct stepper_hw_properties_s {
        uint16_t stepsPerRev       = CONF_HW_MOTOR_STEPS_PER_REV_DEFAULT;
        double screwPitchMm        = CONF_HW_SCREW_PITCH_MM_DEFAULT;
        double maxVelocityMmS      = CONF_HW_MAX_VELOCITY_MM_S_DEFAULT;
        double maxAccelerationMmS2 = CONF_HW_MAX_ACCELERATION_MM_S2_DEFAULT;
        uint16_t encoderTopsPerRev = CONF_HW_ENCODER_TOPS_PER_REV_DEFAULT;
    } stepper_hw_properties_t;

    typedef struct dc_motor_hw_properties_s {
        double screwPitchMm        = 0.0;
        double maxVelocityMmS      = 0.0;
        double maxAccelerationMmS2 = 0.0;
    } dc_motor_hw_properties_t;

    using motor_hw_properties_t = std::variant<
        std::monostate,
        stepper_hw_properties_t,
        dc_motor_hw_properties_t>;

    typedef struct motor_config_s {
        std::string id;
        motor_hw_properties_t hwProperties;
        // TODO: add optional encoder hw props instead of integrating it into the specific motor hw props
    } motor_config_t;

    typedef struct camera_config_s {
        std::string id;
        std::string serialNumber;
        double maxExposureUs     = CONF_HW_MAX_EXPOSURE_US_DEFAULT;
        double defaultExposureUs = CONF_HW_DEFAULT_EXPOSURE_US_DEFAULT;
        double maxGainDb         = CONF_HW_MAX_GAIN_DB_DEFAULT;
        double defaultGainDb     = CONF_HW_DEFAULT_GAIN_DB_DEFAULT;
        double framerate         = CONF_HW_FRAMERATE_DEFAULT;

        // Optional peripheral mappings
        Optional<QString> associatedFocalId = std::nullopt;
        Optional<QString> associatedLightId = std::nullopt;
    } camera_config_t;

    // Top level struct for hardware config
    typedef struct hardware_config_s {
        QString keyboardLayout              = CONF_HW_LOCALE_DEFAULT;
        mcu_hw_properties_t mcus[MCU_COUNT] = {
            {.port = "/dev/arduino1"},
            {.port = "/dev/arduino2"},
            {.port = "/dev/arduino3"},
            {.port = "/dev/arduino4"},
        };
        std::unordered_map<QString, motor_config_t> motors;
        std::unordered_map<QString, camera_config_t> cameras;
        std::unordered_map<QString, double> adc_to_gf_factors;

        void mergeMissingFieldsFrom(const hardware_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            for (const auto &[key, source_motor] : source.motors)
            {
                if (motors.find(key) == motors.end())
                {
                    motors[key] = source_motor;
                    logs.push_back(std::format("{}/motors[{}] = <created from defaults>", path, key.toStdString()));
                }
            }

            for (const auto &[key, source_camera] : source.cameras)
            {
                if (cameras.find(key) == cameras.end())
                {
                    cameras[key] = source_camera;
                    logs.push_back(std::format("{}/cameras[{}] = <created from defaults>", path, key.toStdString()));
                }
            }

            for (const auto &[key, source_val] : source.adc_to_gf_factors)
            {
                if (adc_to_gf_factors.find(key) == adc_to_gf_factors.end())
                {
                    adc_to_gf_factors[key] = source_val;
                    logs.push_back(std::format("{}/adc_to_gf_factors[{}] = {}", path, key.toStdString(), source_val));
                }
            }
        }
    } hardware_config_t;

} // Kub3::Config
