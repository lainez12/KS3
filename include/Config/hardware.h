#pragma once

#include <QString>
#include <string>
#include <unordered_map>
#include <variant>

#define MCU_COUNT 4

namespace Kub3::Config
{

    ///////////////////////////
    // HARDWARE CONFIGURATION
    ///////////////////////////

    typedef struct mcu_hw_properties_s {
        QString port;
        uint32_t baudrate = 115200;
    } mcu_hw_properties_t;

    typedef struct stepper_hw_properties_s {
        uint16_t stepsPerRev;
        double screwPitchMm;
        double maxVelocityMmS;
        double maxAccelerationMmS2;
        uint16_t encoderTopsPerRev;
    } stepper_hw_properties_t;

    typedef struct dc_motor_hw_properties_s {
        double screwPitchMm;
        double maxVelocityMmS;
        double maxAccelerationMmS2;
        uint16_t encoderTopsPerRev;
    } dc_motor_hw_properties_t;

    using motor_hw_properties_t = std::variant<
        std::monostate,
        stepper_hw_properties_t,
        dc_motor_hw_properties_t>;

    typedef struct motor_config_s {
        std::string id;
        motor_hw_properties_t hwProperties;
    } motor_config_t;

    typedef struct camera_config_s {
        std::string id;
        std::string serialNumber;
        double maxExposureUs;
        double defaultExposureUs;
        double maxGainDb;
        double defaultGainDb;
        double framerate = 30.0;
    } camera_config_t;

    // Top level struct for hardware config
    typedef struct hardware_config_s {
        mcu_hw_properties_t mcus[MCU_COUNT];
        std::unordered_map<QString, motor_config_t> motors;
        std::unordered_map<QString, camera_config_t> cameras;
        std::unordered_map<QString, double> adc_to_gf_factors;
    } hardware_config_t;

} // Kub3::Config