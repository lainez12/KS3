#include <QDebug>
#include <QSettings>
#include <QString>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <variant>

#include <Config/ConfigSaver.h>
#include <Config/keys/admin.h>
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

            // =============================
            // LOAD MCUs SETTINGS
            // =============================
            settings.beginGroup(CONF_HW_MCUS);
            {
#if defined(KUB_MODEL_8)
                static_assert(MCU_COUNT == 4, "MCU_COUNT does not match the model expected value (expected: 4)");

                // Ports
                settings.setValue(CONF_HW_MCU1_PORT, config.mcus[0].port);
                settings.setValue(CONF_HW_MCU2_PORT, config.mcus[1].port);
                settings.setValue(CONF_HW_MCU3_PORT, config.mcus[2].port);
                settings.setValue(CONF_HW_MCU4_PORT, config.mcus[3].port);
                // Baudrates
                settings.setValue(CONF_HW_MCU1_BAUDRATE, config.mcus[0].baudrate);
                settings.setValue(CONF_HW_MCU2_BAUDRATE, config.mcus[1].baudrate);
                settings.setValue(CONF_HW_MCU3_BAUDRATE, config.mcus[2].baudrate);
                settings.setValue(CONF_HW_MCU4_BAUDRATE, config.mcus[3].baudrate);
#endif
            }
            settings.endGroup();

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
                settings.setValue(CONF_HW_FRAMERATE, camera.framerate);

                // Optional associated hardware
                settings.setValue(CONF_HW_ASSOCIATED_FOCAL_ID, camera.associatedFocalId.value_or(""));
                settings.setValue(CONF_HW_ASSOCIATED_LIGHT_ID, camera.associatedLightId.value_or(""));

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
            settings.setValue(CONF_PROCESS_MIN_CAMERA_DISTANCE_MM, config.vision.min_camera_distance_mm);
            // --- Cameras' init
            settings.setValue(CONF_PROCESS_LEFT_CAM_X_RESET_POS_MM, config.vision.left_cam_x_reset_pos_mm);
            settings.setValue(CONF_PROCESS_LEFT_CAM_Y_RESET_POS_MM, config.vision.left_cam_y_reset_pos_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_X_RESET_POS_MM, config.vision.right_cam_x_reset_pos_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_Y_RESET_POS_MM, config.vision.right_cam_y_reset_pos_mm);
            // --- Cameras' homing
            settings.setValue(CONF_PROCESS_LEFT_CAM_X_HOME_POS_MM, config.vision.left_cam_x_home_pos_mm);
            settings.setValue(CONF_PROCESS_LEFT_CAM_Y_HOME_POS_MM, config.vision.left_cam_y_home_pos_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_X_HOME_POS_MM, config.vision.right_cam_x_home_pos_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_Y_HOME_POS_MM, config.vision.right_cam_y_home_pos_mm);
            // Cameras' virtual limits
            settings.setValue(CONF_PROCESS_LEFT_CAM_X_VIRTUAL_LIMIT_POS_MM, config.vision.left_cam_x_virtual_limit_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_X_VIRTUAL_LIMIT_POS_MM, config.vision.right_cam_x_virtual_limit_mm);
            // --- Focals
            {
                settings.beginGroup(CONF_PROCESS_LEFT_FOCAL);
                settings.setValue(CONF_PROCESS_FOCAL_DEFAULT_VALUE, config.vision.left_focal_conf.default_value);
                settings.setValue(CONF_PROCESS_FOCAL_MIN_VALUE, config.vision.left_focal_conf.min_value);
                settings.setValue(CONF_PROCESS_FOCAL_MAX_VALUE, config.vision.left_focal_conf.max_value);
                settings.endGroup();
                settings.beginGroup(CONF_PROCESS_RIGHT_FOCAL);
                settings.setValue(CONF_PROCESS_FOCAL_DEFAULT_VALUE, config.vision.right_focal_conf.default_value);
                settings.setValue(CONF_PROCESS_FOCAL_MIN_VALUE, config.vision.right_focal_conf.min_value);
                settings.setValue(CONF_PROCESS_FOCAL_MAX_VALUE, config.vision.right_focal_conf.max_value);
                settings.endGroup();
            }
            settings.endGroup(); // CONF_PROCESS_CAMERAS

            // SAVE ALIGNMENT POSITIONS
            settings.beginGroup(CONF_PROCESS_ALIGNMENT_POSITIONS);
            settings.setValue(CONF_PROCESS_X_STAGE_CENTER_POS_MM, config.alignment.x_stage_center_pos_mm);
            settings.setValue(CONF_PROCESS_Y_STAGE_CENTER_POS_MM, config.alignment.y_stage_center_pos_mm);
            settings.setValue(CONF_PROCESS_THETA_STAGE_CENTER_POS_MM, config.alignment.theta_stage_center_pos_mm);
            settings.endGroup(); // CONF_PROCESS_ALIGNMENT_POSITIONS

            // SAVE Z ELEVATOR SETTINGS
            settings.beginGroup(CONF_PROCESS_ELEVATOR_POSITIONS);
            settings.setValue(CONF_PROCESS_MAX_Z_RELATIVE_DISTANCE_MM, config.elevator.max_z_relative_distance_mm);
            settings.endGroup();

            // SAVE DRAWERS POSITIONS
            settings.beginGroup(CONF_PROCESS_DRAWERS_POSITIONS);
            settings.setValue(CONF_PROCESS_CM3_RESET_POS_MM, config.drawers.cm3_reset_pos_mm);
            settings.endGroup(); // CONF_PROCESS_DRAWERS_POSITIONS

            // SAVE FORCE LIMITS
            settings.beginGroup(CONF_PROCESS_FORCE_LIMITS);
            settings.setValue(CONF_PROCESS_HW_CRASH_FORCE_LIMIT_GF, config.contact.hw_crash_force_limit_gf);
            settings.setValue(CONF_PROCESS_MAX_FORCE_GF, config.contact.max_process_force_gf);
            settings.setValue(CONF_PROCESS_CONTACT_THRESHOLD_GF, config.contact.contact_threshold_gf);
            settings.setValue(CONF_PROCESS_AUTOLEVEL_FORCE_GF, config.contact.autolevel_force_gf);
            settings.setValue(CONF_PROCESS_AUTOLEVEL_FORCE_TOLERANCE_GF, config.contact.autolevel_force_tolerance_gf);
            settings.endGroup(); // CONF_PROCESS_FORCE_LIMITS

            // SAVE ADMITTANCE TUNING VALUES
            settings.beginGroup(CONF_PROCESS_ADMITTANCE_TUNING);
            settings.setValue(CONF_PROCESS_ADMITTANCE_MAX_STEP_MM_PER_TICK, config.contact.admittance.max_step_mm_per_tick);
            settings.setValue(CONF_PROCESS_ADMITTANCE_DEADBAND_VELOCITY_MM_S, config.contact.admittance.deadband_velocity_mm_s);
            settings.setValue(CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_LOW_FORCE, config.contact.admittance.translational_gain_low_force);
            settings.setValue(CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_HIGH_FORCE, config.contact.admittance.translational_gain_high_force);
            settings.setValue(CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_LOW_FORCE, config.contact.admittance.rotational_gain_low_force);
            settings.setValue(CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_HIGH_FORCE, config.contact.admittance.rotational_gain_high_force);
            settings.endGroup(); // CONF_PROCESS_ADMITTANCE_TUNING

            // PAD MOVEMENTS VALUES
            settings.beginGroup(CONF_PROCESS_PAD_MOVEMENTS);
            settings.setValue(CONF_PROCESS_X_STAGE_DISTANCE_MM, config.pad.x_stage_distance_mm);
            settings.setValue(CONF_PROCESS_Y_STAGE_DISTANCE_MM, config.pad.y_stage_distance_mm);
            settings.setValue(CONF_PROCESS_THETA_STAGE_DISTANCE_MM, config.pad.theta_stage_distance_mm);
            settings.setValue(CONF_PROCESS_Z_MOTORS_DISTANCE_MM, config.pad.z_motors_distance_mm);
            settings.setValue(CONF_PROCESS_LEFT_CAM_X_DISTANCE_MM, config.pad.left_cam_x_distance_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_X_DISTANCE_MM, config.pad.right_cam_x_distance_mm);
            settings.setValue(CONF_PROCESS_LEFT_CAM_Y_DISTANCE_MM, config.pad.left_cam_y_distance_mm);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_Y_DISTANCE_MM, config.pad.right_cam_y_distance_mm);
            settings.setValue(CONF_PROCESS_LEFT_CAM_LIGHTING_STEP_PC, config.pad.left_cam_light_step_pc);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_LIGHTING_STEP_PC, config.pad.right_cam_light_step_pc);
            settings.setValue(CONF_PROCESS_LEFT_CAM_FOCAL_STEP_PC, config.pad.left_cam_focal_step_pc);
            settings.setValue(CONF_PROCESS_RIGHT_CAM_FOCAL_STEP_PC, config.pad.right_cam_focal_step_pc);
            settings.endGroup();

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

    void ConfigSaver::saveAdminConfig(const admin_config_t &config, const std::string &targetPath)
    {
        qInfo() << "[SAVING ADMIN CONFIGURATION] to" << targetPath.c_str();
        const std::string tempPath = targetPath + ".tmp";

        {
            // BEGIN SCOPE: Ensure QSettings flushes and releases lock
            QSettings settings(QString::fromStdString(tempPath), QSettings::IniFormat);

            settings.setValue(CONF_ADMIN_KLOE_MODE, config.kloe_mode);

            // Flush and verify write
            settings.sync();
            if (settings.status() != QSettings::NoError)
            {
                throw std::runtime_error(std::format("CRITICAL: Failed to write temp admin config file: {}", tempPath));
            }
        } // END SCOPE

        // ATOMIC SWAP
        std::error_code ec;
        std::filesystem::rename(tempPath, targetPath, ec);
        if (ec)
        {
            std::filesystem::remove(tempPath, ec); // Cleanup orphaned tmp file
            throw std::runtime_error(std::format("CRITICAL: Atomic admin config swap failed: {}", ec.message()));
        }
    }

} // namespace Kub3::Config
