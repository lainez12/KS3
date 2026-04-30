#include <QDebug>
#include <QSettings>
#include <QString>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <variant>

#include <Config/ConfigSaver.h>
#include <Config/keys/hardware.h>
#include <Config/keys/process.h>

namespace Kub3::Config
{

    void ConfigSaver::saveHardwareConfig(const hardware_config_t &config, const std::string &targetPath)
    {
        qInfo() << "[SAVING HARDWARE CONFIGURATION] to" << targetPath.c_str();
        const std::string tempPath = targetPath + ".tmp";

        {
            // BEGIN SCOPE: Ensure QSettings flushes to OS buffers and releases file handles
            QSettings settings(QString::fromStdString(tempPath), QSettings::IniFormat);

            // SAVE MOTORS PARAMETERS
            settings.beginGroup(CONF_MOTORS);
            for (const auto &[id, motor] : config.motors)
            {
                settings.beginGroup(id);

                if (std::holds_alternative<stepper_hw_properties_t>(motor.hwProperties))
                {
                    const auto &hw = std::get<stepper_hw_properties_t>(motor.hwProperties);
                    settings.setValue(CONF_MOTOR_TYPE, CONF_MOTOR_TYPE_STEPPER);
                    settings.setValue(CONF_MOTOR_STEPS_PER_REV, hw.stepsPerRev);
                    settings.setValue(CONF_SCREW_PITCH_MM, hw.screwPitchMm);
                    settings.setValue(CONF_MAX_VELOCITY_MM_S, hw.maxVelocityMmS);
                    settings.setValue(CONF_MAX_ACCELERATION_MM_S2, hw.maxAccelerationMmS2);
                    settings.setValue(CONF_ENCODER_TOPS_PER_REV, hw.encoderTopsPerRev);
                }
                else if (std::holds_alternative<dc_motor_hw_properties_t>(motor.hwProperties))
                {
                    const auto &hw = std::get<dc_motor_hw_properties_t>(motor.hwProperties);
                    settings.setValue(CONF_MOTOR_TYPE, CONF_MOTOR_TYPE_DC);
                    settings.setValue(CONF_SCREW_PITCH_MM, hw.screwPitchMm);
                    settings.setValue(CONF_MAX_VELOCITY_MM_S, hw.maxVelocityMmS);
                    settings.setValue(CONF_MAX_ACCELERATION_MM_S2, hw.maxAccelerationMmS2);
                    settings.setValue(CONF_ENCODER_TOPS_PER_REV, hw.encoderTopsPerRev);
                }

                settings.endGroup(); // motor.id
            }
            settings.endGroup(); // CONF_MOTORS

            // SAVE CAMERAS PARAMETERS
            settings.beginGroup(CONF_CAMERAS);
            for (const auto &[id, camera] : config.cameras)
            {
                settings.beginGroup(id);
                settings.setValue(CONF_SERIAL_NUMBER, QString::fromStdString(camera.serialNumber));
                settings.setValue(CONF_MAX_EXPOSURE_US, camera.maxExposureUs);
                settings.setValue(CONF_DEFAULT_EXPOSURE_US, camera.defaultExposureUs);
                settings.setValue(CONF_MAX_GAIN_DB, camera.maxGainDb);
                settings.setValue(CONF_DEFAULT_GAIN_DB, camera.defaultGainDb);
                settings.endGroup(); // camera.id
            }
            settings.endGroup(); // CONF_CAMERAS

            // SAVE FORCE SENSORS PARAMETERS
            settings.beginGroup(CONF_FORCE_SENSORS);
            for (const auto &[id, factor] : config.adc_to_gf_factors)
            {
                settings.beginGroup(id);
                settings.setValue(ADC_TO_GRAM_FORCE_FACTOR, factor);
                settings.endGroup(); // sensor id
            }
            settings.endGroup(); // "forceSensors"

            // Flush and verify write
            settings.sync();
            if (settings.status() != QSettings::NoError)
            {
                throw std::runtime_error(std::format("CRITICAL: Failed to write temp hardware config file: {}", tempPath));
            }
        } // END SCOPE

        // ATOMIC SWAP
        std::error_code ec;
        std::filesystem::rename(tempPath, targetPath, ec);
        if (ec)
        {
            std::filesystem::remove(tempPath, ec); // Cleanup orphaned tmp file
            throw std::runtime_error(std::format("CRITICAL: Atomic hardware config swap failed: {}", ec.message()));
        }
    }

    void ConfigSaver::saveProcessConfig(const process_config_t &config, const std::string &targetPath)
    {
        qInfo() << "[SAVING PROCESS CONFIGURATION] to" << targetPath.c_str();
        const std::string tempPath = targetPath + ".tmp";

        {
            // BEGIN SCOPE: Ensure QSettings flushes and releases lock
            QSettings settings(QString::fromStdString(tempPath), QSettings::IniFormat);

            // SAVE KINEMATIC PROFILES
            settings.beginGroup(CONF_KINEMATICS);
            for (const auto &[motorId, profilesMap] : config.kinematic_profiles)
            {
                settings.beginGroup(QString::fromStdString(motorId));

                for (const auto &[profileId, profile] : profilesMap)
                {
                    settings.beginGroup(QString::fromStdString(profileId));

                    settings.setValue(CONF_INITIAL_VELOCITY_MM_S, profile.initialVelocityMmS);
                    settings.setValue(CONF_TARGET_VELOCITY_MM_S, profile.targetVelocityMmS);
                    settings.setValue(CONF_ACCELERATION_MM_S, profile.accelerationMmS2); // Note: Loader uses acceleration_mm_s

                    if (std::holds_alternative<stepper_kinematics_params_t>(profile.params))
                    {
                        const auto &params = std::get<stepper_kinematics_params_t>(profile.params);
                        settings.setValue(CONF_PARAMS_TYPE, CONF_PARAMS_TYPE_STEPPER);
                        settings.setValue(CONF_STEP_FRACTION, params.stepFraction);
                    }

                    settings.endGroup(); // profileId
                }
                settings.endGroup(); // motorId
            }
            settings.endGroup(); // CONF_KINEMATICS

            // SAVE CAMERAS DATA
            settings.beginGroup(CONF_CAMERAS);
            settings.setValue(CONF_MIN_CAMERA_DISTANCE_MM, config.min_camera_distance_mm);
            settings.endGroup(); // CONF_CAMERAS

            // SAVE FORCE LIMITS
            settings.beginGroup(CONF_FORCE_LIMITS);
            settings.setValue(CONF_HW_CRASH_FORCE_LIMIT_GF, config.hw_crash_force_limit_gf);
            settings.setValue(CONF_MAX_FORCE_GF, config.max_force_gf);
            settings.setValue(CONF_CONTACT_THRESHOLD_GF, config.contact_threshold_gf);
            settings.endGroup(); // CONF_FORCE_LIMITS

            // Flush and verify write
            settings.sync();
            if (settings.status() != QSettings::NoError)
            {
                throw std::runtime_error(std::format("CRITICAL: Failed to write temp process config file: {}", tempPath));
            }
        } // END SCOPE

        // ATOMIC SWAP
        std::error_code ec;
        std::filesystem::rename(tempPath, targetPath, ec);
        if (ec)
        {
            std::filesystem::remove(tempPath, ec); // Cleanup orphaned tmp file
            throw std::runtime_error(std::format("CRITICAL: Atomic process config swap failed: {}", ec.message()));
        }
    }

} // namespace Kub3::Config
