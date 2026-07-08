#pragma once

#include <QString>
#include <string>
#include <unordered_map>
#include <variant>

#include "../utils.h" // @note: keep relative path for compilation
#include <Config/utils.h>

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

        void mergeMissingFieldsFrom(const mcu_hw_properties_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_QSTRING_FIELD(*this, source, port, path, logs);
            MERGE_FIELD(*this, source, 115200, baudrate, path, logs);
        }

    } mcu_hw_properties_t;

    typedef struct stepper_hw_properties_s {
        uint16_t stepsPerRev;
        double screwPitchMm;
        double maxVelocityMmS;
        double maxAccelerationMmS2;
        uint16_t encoderTopsPerRev;

        void mergeMissingFieldsFrom(const stepper_hw_properties_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0, stepsPerRev, path, logs);
            MERGE_FIELD(*this, source, 0.0, screwPitchMm, path, logs);
            MERGE_FIELD(*this, source, 0.0, maxVelocityMmS, path, logs);
            MERGE_FIELD(*this, source, 0.0, maxAccelerationMmS2, path, logs);
            MERGE_FIELD(*this, source, 0, encoderTopsPerRev, path, logs);
        }

    } stepper_hw_properties_t;

    typedef struct dc_motor_hw_properties_s {
        double screwPitchMm;
        double maxVelocityMmS;
        double maxAccelerationMmS2;

        void mergeMissingFieldsFrom(const dc_motor_hw_properties_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0.0, screwPitchMm, path, logs);
            MERGE_FIELD(*this, source, 0.0, maxVelocityMmS, path, logs);
            MERGE_FIELD(*this, source, 0.0, maxAccelerationMmS2, path, logs);
        }

    } dc_motor_hw_properties_t;

    using motor_hw_properties_t = std::variant<
        std::monostate,
        stepper_hw_properties_t,
        dc_motor_hw_properties_t>;

    typedef struct motor_config_s {
        std::string id;
        motor_hw_properties_t hwProperties;
        // TODO: add optional encoder hw props instead of integrating it into the specific motor hw props

        void mergeMissingFieldsFrom(const motor_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_STRING_FIELD(*this, source, id, path, logs);

            if (std::holds_alternative<std::monostate>(hwProperties) && !std::holds_alternative<std::monostate>(source.hwProperties))
            {
                hwProperties = source.hwProperties;
                logs.push_back(std::format("{}/hwProperties = <created from defaults>", path));
            }
            else if (std::holds_alternative<stepper_hw_properties_t>(hwProperties) && std::holds_alternative<stepper_hw_properties_t>(source.hwProperties))
            {
                std::get<stepper_hw_properties_t>(hwProperties).mergeMissingFieldsFrom(std::get<stepper_hw_properties_t>(source.hwProperties), std::format("{}/hwProperties", path), logs);
            }
            else if (std::holds_alternative<dc_motor_hw_properties_t>(hwProperties) && std::holds_alternative<dc_motor_hw_properties_t>(source.hwProperties))
            {
                std::get<dc_motor_hw_properties_t>(hwProperties).mergeMissingFieldsFrom(std::get<dc_motor_hw_properties_t>(source.hwProperties), std::format("{}/hwProperties", path), logs);
            }
        }

    } motor_config_t;

    typedef struct camera_config_s {
        std::string id;
        std::string serialNumber;
        double maxExposureUs;
        double defaultExposureUs;
        double maxGainDb;
        double defaultGainDb;
        double framerate = 30.0;

        // Optional peripheral mappings
        Optional<QString> associatedFocalId;
        Optional<QString> associatedLightId;

        void mergeMissingFieldsFrom(const camera_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_STRING_FIELD(*this, source, id, path, logs);
            MERGE_STRING_FIELD(*this, source, serialNumber, path, logs);
            MERGE_FIELD(*this, source, 0.0, maxExposureUs, path, logs);
            MERGE_FIELD(*this, source, 0.0, defaultExposureUs, path, logs);
            MERGE_FIELD(*this, source, 0.0, maxGainDb, path, logs);
            MERGE_FIELD(*this, source, 0.0, defaultGainDb, path, logs);
            MERGE_FIELD(*this, source, 30.0, framerate, path, logs);

            MERGE_OPTIONAL_QSTRING_FIELD(*this, source, associatedFocalId, path, logs);
            MERGE_OPTIONAL_QSTRING_FIELD(*this, source, associatedLightId, path, logs);
        }

    } camera_config_t;

    // Top level struct for hardware config
    typedef struct hardware_config_s {
        mcu_hw_properties_t mcus[MCU_COUNT];
        std::unordered_map<QString, motor_config_t> motors;
        std::unordered_map<QString, camera_config_t> cameras;
        std::unordered_map<QString, double> adc_to_gf_factors;

        void mergeMissingFieldsFrom(const hardware_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            for (size_t i = 0; i < MCU_COUNT; ++i)
            {
                mcus[i].mergeMissingFieldsFrom(source.mcus[i], std::format("{}/mcus[{}]", path, i), logs);
            }

            for (const auto &[key, source_motor] : source.motors)
            {
                std::string sub_path = std::format("{}/motors[{}]", path, key.toStdString());
                if (motors.find(key) == motors.end())
                {
                    motors[key] = source_motor;
                    logs.push_back(std::format("{} = <created from defaults>", sub_path));
                }
                else
                {
                    motors[key].mergeMissingFieldsFrom(source_motor, sub_path, logs);
                }
            }

            for (const auto &[key, source_camera] : source.cameras)
            {
                std::string sub_path = std::format("{}/cameras[{}]", path, key.toStdString());
                if (cameras.find(key) == cameras.end())
                {
                    cameras[key] = source_camera;
                    logs.push_back(std::format("{} = <created from defaults>", sub_path));
                }
                else
                {
                    cameras[key].mergeMissingFieldsFrom(source_camera, sub_path, logs);
                }
            }

            for (const auto &[key, source_val] : source.adc_to_gf_factors)
            {
                std::string sub_path = std::format("{}/adc_to_gf_factors[{}]", path, key.toStdString());
                if (adc_to_gf_factors.find(key) == adc_to_gf_factors.end())
                {
                    adc_to_gf_factors[key] = source_val;
                    logs.push_back(std::format("{} = {}", sub_path, source_val));
                }
            }
        }
    } hardware_config_t;

} // Kub3::Config