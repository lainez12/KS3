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
            settings.beginGroup(CONF_HW_MOTORS);
            for (const auto &[id, motor] : config.motors)
            {
                settings.beginGroup(id);

                if (std::holds_alternative<stepper_hw_properties_t>(motor.hwProperties))
                {
                    const auto &hw = std::get<stepper_hw_properties_t>(motor.hwProperties);
                    settings.setValue(CONF_HW_MOTOR_TYPE, CONF_HW_MOTOR_TYPE_STEPPER);
                    settings.setValue(CONF_HW_MOTOR_STEPS_PER_REV, hw.stepsPerRev);
                    settings.setValue(CONF_HW_SCREW_PITCH_MM, hw.screwPitchMm);
                    settings.setValue(CONF_HW_MAX_VELOCITY_MM_S, hw.maxVelocityMmS);
                    settings.setValue(CONF_HW_MAX_ACCELERATION_MM_S2, hw.maxAccelerationMmS2);
                    settings.setValue(CONF_HW_ENCODER_TOPS_PER_REV, hw.encoderTopsPerRev);
                }
                else if (std::holds_alternative<dc_motor_hw_properties_t>(motor.hwProperties))
                {
                    const auto &hw = std::get<dc_motor_hw_properties_t>(motor.hwProperties);
                    settings.setValue(CONF_HW_MOTOR_TYPE, CONF_HW_MOTOR_TYPE_DC);
                    settings.setValue(CONF_HW_SCREW_PITCH_MM, hw.screwPitchMm);
                    settings.setValue(CONF_HW_MAX_VELOCITY_MM_S, hw.maxVelocityMmS);
                    settings.setValue(CONF_HW_MAX_ACCELERATION_MM_S2, hw.maxAccelerationMmS2);
                    settings.setValue(CONF_HW_ENCODER_TOPS_PER_REV, hw.encoderTopsPerRev);
                }

                settings.endGroup(); // motor.id
            }
            settings.endGroup(); // CONF_HW_MOTORS

            // SAVE CAMERAS PARAMETERS
            settings.beginGroup(CONF_HW_CAMERAS);
            for (const auto &[id, camera] : config.cameras)
            {
                settings.beginGroup(id);
                settings.setValue(CONF_HW_SERIAL_NUMBER, QString::fromStdString(camera.serialNumber));
                settings.setValue(CONF_HW_MAX_EXPOSURE_US, camera.maxExposureUs);
                settings.setValue(CONF_HW_DEFAULT_EXPOSURE_US, camera.defaultExposureUs);
                settings.setValue(CONF_HW_MAX_GAIN_DB, camera.maxGainDb);
                settings.setValue(CONF_HW_DEFAULT_GAIN_DB, camera.defaultGainDb);
                settings.endGroup(); // camera.id
            }
            settings.endGroup(); // CONF_HW_CAMERAS

            // SAVE FORCE SENSORS PARAMETERS
            settings.beginGroup(CONF_HW_FORCE_SENSORS);
            for (const auto &[id, factor] : config.adc_to_gf_factors)
            {
                settings.beginGroup(id);
                settings.setValue(CONF_HW_ADC_TO_GRAM_FORCE_FACTOR, factor);
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
            settings.beginGroup(CONF_PROCESS_KINEMATICS);
            for (const auto &[motorId, profilesMap] : config.kinematic_profiles)
            {
                settings.beginGroup(QString::fromStdString(motorId));

                for (const auto &[profileId, profile] : profilesMap)
                {
                    settings.beginGroup(QString::fromStdString(profileId));

                    settings.setValue(CONF_PROCESS_INITIAL_VELOCITY_MM_S, profile.initialVelocityMmS);
                    settings.setValue(CONF_PROCESS_TARGET_VELOCITY_MM_S, profile.targetVelocityMmS);
                    settings.setValue(CONF_PROCESS_ACCELERATION_MM_S, profile.accelerationMmS2); // Note: Loader uses acceleration_mm_s

                    if (std::holds_alternative<stepper_kinematics_params_t>(profile.params))
                    {
                        const auto &params = std::get<stepper_kinematics_params_t>(profile.params);
                        settings.setValue(CONF_PROCESS_PARAMS_TYPE, CONF_PROCESS_PARAMS_TYPE_STEPPER);
                        settings.setValue(CONF_PROCESS_STEP_FRACTION, params.stepFraction);
                    }
                    else
                    {
                        settings.setValue(CONF_PROCESS_PARAMS_TYPE, CONF_PROCESS_PARAMS_TYPE_GENERIC);
                    }

                    settings.endGroup(); // profileId
                }
                settings.endGroup(); // motorId
            }
            settings.endGroup(); // CONF_PROCESS_KINEMATICS

            // SAVE CAMERAS DATA
            settings.beginGroup(CONF_PROCESS_CAMERAS);
            settings.setValue(CONF_PROCESS_MIN_CAMERA_DISTANCE_MM, config.min_camera_distance_mm);
            settings.setValue(CONF_PROCESS_LEFT_CAM_X_RESET_POS_MM, config.left_cam_x_reset_pos_mm);
            settings.setValue(CONF_PROCESS_LEFT_CAM_Y_RESET_POS_MM, config.left_cam_y_reset_pos_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_X_RESET_POS_MM, config.right_cam_x_reset_pos_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_Y_RESET_POS_MM, config.right_cam_y_reset_pos_mm);
            settings.endGroup(); // CONF_PROCESS_CAMERAS

            // SAVE ALIGNMENT POSITIONS
            settings.beginGroup(CONF_PROCESS_ALIGNMENT_POSITIONS);
            settings.setValue(CONF_PROCESS_X_STAGE_CENTER_POS_MM, config.x_stage_center_pos_mm);
            settings.setValue(CONF_PROCESS_Y_STAGE_CENTER_POS_MM, config.y_stage_center_pos_mm);
            settings.setValue(CONF_PROCESS_THETA_STAGE_CENTER_POS_MM, config.theta_stage_center_pos_mm);
            settings.endGroup();

            // SAVE DRAWERS POSITIONS
            settings.beginGroup(CONF_PROCESS_DRAWERS_POSITIONS);
            settings.setValue(CONF_PROCESS_CM3_RESET_POS_MM, config.cm3_reset_pos_mm);
            settings.endGroup();

            // SAVE FORCE LIMITS
            settings.beginGroup(CONF_PROCESS_FORCE_LIMITS);
            settings.setValue(CONF_PROCESS_HW_CRASH_FORCE_LIMIT_GF, config.hw_crash_force_limit_gf);
            settings.setValue(CONF_PROCESS_MAX_FORCE_GF, config.max_force_gf);
            settings.setValue(CONF_PROCESS_CONTACT_THRESHOLD_GF, config.contact_threshold_gf);
            settings.endGroup(); // CONF_PROCESS_FORCE_LIMITS

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
