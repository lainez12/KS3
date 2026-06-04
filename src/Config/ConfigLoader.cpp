#include <QSettings>
#include <QStringList>
#include <format>
#include <stdexcept>

#include <Config/ConfigLoader.h>
#include <Config/keys/hardware.h>
#include <Config/keys/process.h>

#define CHECK_CONFIG_VALUE(cond, elemId, value, valName)                                                               \
    do                                                                                                                 \
    {                                                                                                                  \
        if (cond)                                                                                                      \
        {                                                                                                              \
            throw std::runtime_error(std::format("CRITICAL: Invalid '{}' value for {} ({})", valName, elemId, value)); \
        }                                                                                                              \
    } while (0);

namespace Kub3::Config
{

    // Helper to enforce required keys
    static QVariant getRequiredValue(const QSettings &settings, const QString &key, const std::string &group)
    {
        if (!settings.contains(key))
        {
            throw std::runtime_error(std::format("CRITICAL: Missing config key '{}' in group '{}'", key.toStdString(), group));
        }
        return settings.value(key);
    }

    static QVariant getRequiredValue(const QSettings &settings, const QString &key, const QString &group)
    {
        return getRequiredValue(settings, key, group.toStdString());
    }

    hardware_config_t ConfigLoader::loadHardwareConfig(const std::string &filePath)
    {
        hardware_config_t config;
        QSettings settings(QString::fromStdString(filePath), QSettings::IniFormat);

        if (settings.status() != QSettings::NoError)
        {
            throw std::runtime_error("CRITICAL: Failed to open or parse hardware config file: " + filePath);
        }

        // =============================
        // LOAD MCUs SETTINGS
        // =============================
        settings.beginGroup(CONF_HW_MCUS);
        {
            static_assert(MCU_COUNT == 4, "MCU_COUNT does not match the model expected value (4)");
            const std::string group(CONF_HW_MCUS);

            // Ports
            config.mcus[0] = mcu_hw_properties_s{
                .port     = getRequiredValue(settings, CONF_HW_MCU1_PORT, group).toString(),
                .baudrate = getRequiredValue(settings, CONF_HW_MCU1_BAUDRATE, group).toUInt(),
            };
            config.mcus[1] = mcu_hw_properties_s{
                .port     = getRequiredValue(settings, CONF_HW_MCU2_PORT, group).toString(),
                .baudrate = getRequiredValue(settings, CONF_HW_MCU2_BAUDRATE, group).toUInt(),
            };
            config.mcus[2] = mcu_hw_properties_s{
                .port     = getRequiredValue(settings, CONF_HW_MCU3_PORT, group).toString(),
                .baudrate = getRequiredValue(settings, CONF_HW_MCU3_BAUDRATE, group).toUInt(),
            };
            config.mcus[3] = mcu_hw_properties_s{
                .port     = getRequiredValue(settings, CONF_HW_MCU4_PORT, group).toString(),
                .baudrate = getRequiredValue(settings, CONF_HW_MCU4_BAUDRATE, group).toUInt(),
            };
        }
        settings.endGroup();

        // =============================
        // LOAD MOTORS PARAMETERS
        // =============================
        settings.beginGroup(CONF_HW_MOTORS);
        for (const QString &group : settings.childGroups())
        {
            settings.beginGroup(group);

            motor_config_t motor;
            motor.id = group.toStdString();

            QString type = getRequiredValue(settings, CONF_HW_MOTOR_TYPE, motor.id).toString();

            if (type == CONF_HW_MOTOR_TYPE_STEPPER)
            {
                stepper_hw_properties_t hw;

                // Parse, Don't Validate: We immediately cast to correct type. If it's malformed, it throws.
                hw.stepsPerRev         = getRequiredValue(settings, CONF_HW_MOTOR_STEPS_PER_REV, motor.id).toUInt();
                hw.screwPitchMm        = getRequiredValue(settings, CONF_HW_SCREW_PITCH_MM, motor.id).toDouble();
                hw.maxVelocityMmS      = getRequiredValue(settings, CONF_HW_MAX_VELOCITY_MM_S, motor.id).toDouble();
                hw.maxAccelerationMmS2 = getRequiredValue(settings, CONF_HW_MAX_ACCELERATION_MM_S2, motor.id).toDouble();
                hw.encoderTopsPerRev   = getRequiredValue(settings, CONF_HW_ENCODER_TOPS_PER_REV, motor.id).toUInt();

                CHECK_CONFIG_VALUE(hw.stepsPerRev == 0.0, motor.id, hw.stepsPerRev, "steps per revolution");
                CHECK_CONFIG_VALUE(hw.screwPitchMm == 0.0, motor.id, hw.screwPitchMm, "screw pitch");
                CHECK_CONFIG_VALUE(hw.encoderTopsPerRev == 0.0, motor.id, hw.encoderTopsPerRev, "encoder tops per revolution");

                motor.hwProperties = hw;
            }
            else if (type == CONF_HW_MOTOR_TYPE_DC)
            {
                dc_motor_hw_properties_t hw;

                hw.screwPitchMm        = getRequiredValue(settings, CONF_HW_SCREW_PITCH_MM, motor.id).toDouble();
                hw.maxVelocityMmS      = getRequiredValue(settings, CONF_HW_MAX_VELOCITY_MM_S, motor.id).toDouble();
                hw.maxAccelerationMmS2 = getRequiredValue(settings, CONF_HW_MAX_ACCELERATION_MM_S2, motor.id).toDouble();
                hw.encoderTopsPerRev   = getRequiredValue(settings, CONF_HW_ENCODER_TOPS_PER_REV, motor.id).toUInt();

                CHECK_CONFIG_VALUE(hw.screwPitchMm == 0.0, motor.id, hw.screwPitchMm, "screw pitch");
                CHECK_CONFIG_VALUE(hw.encoderTopsPerRev == 0.0, motor.id, hw.encoderTopsPerRev, "encoder tops per revolution");

                motor.hwProperties = hw;
            }
            else
            {
                throw std::runtime_error(std::format("CRITICAL: Unknown motor type '{}' for '{}'", type.toStdString(), motor.id));
            }

            config.motors.emplace(group, motor);
            settings.endGroup();
        }
        settings.endGroup(); // CONF_HW_MOTORS

        // =============================
        // LOAD CAMERAS PARAMETERS
        // =============================
        settings.beginGroup(CONF_HW_CAMERAS);
        for (const QString &group : settings.childGroups())
        {
            settings.beginGroup(group);

            camera_config_t camera = {
                .id                = group.toStdString(),
                .serialNumber      = getRequiredValue(settings, CONF_HW_SERIAL_NUMBER, group).toString().toStdString(),
                .maxExposureUs     = getRequiredValue(settings, CONF_HW_MAX_EXPOSURE_US, group).toDouble(),
                .defaultExposureUs = getRequiredValue(settings, CONF_HW_DEFAULT_EXPOSURE_US, group).toDouble(),
                .maxGainDb         = getRequiredValue(settings, CONF_HW_MAX_GAIN_DB, group).toDouble(),
                .defaultGainDb     = getRequiredValue(settings, CONF_HW_DEFAULT_GAIN_DB, group).toDouble(),
            };

            config.cameras.emplace(group, camera);
            settings.endGroup(); // group
        }
        settings.endGroup(); // CONF_HW_CAMERAS

        // =============================
        // FORCE LIMITS
        // =============================
        settings.beginGroup(CONF_HW_FORCE_SENSORS);
        for (const QString &group : settings.childGroups())
        {
            settings.beginGroup(group);
            config.adc_to_gf_factors.emplace(group, getRequiredValue(settings, CONF_HW_ADC_TO_GRAM_FORCE_FACTOR, group).toDouble());
            settings.endGroup(); // group
        }
        settings.endGroup(); // CONF_HW_FORCE_SENSORS

        return config;
    }

    process_config_t ConfigLoader::loadProcessConfig(const std::string &filePath)
    {
        process_config_t config;
        QSettings settings(QString::fromStdString(filePath), QSettings::IniFormat);

        if (settings.status() != QSettings::NoError)
        {
            throw std::runtime_error("CRITICAL: Failed to open or parse process config file: " + filePath);
        }

        // =============================
        // LOAD KINEMATIC PROFILES
        // =============================
        settings.beginGroup(CONF_PROCESS_KINEMATICS);
        for (const QString &motorGroup : settings.childGroups())
        {
            settings.beginGroup(motorGroup);
            const std::string motorId = motorGroup.toStdString();

            for (const QString &profileGroup : settings.childGroups())
            {
                settings.beginGroup(profileGroup);
                kinematic_profile_t profile;

                profile.id                 = profileGroup.toStdString();
                profile.initialVelocityMmS = getRequiredValue(settings, CONF_PROCESS_INITIAL_VELOCITY_MM_S, profile.id).toDouble();
                profile.targetVelocityMmS  = getRequiredValue(settings, CONF_PROCESS_TARGET_VELOCITY_MM_S, profile.id).toDouble();
                profile.accelerationMmS2   = getRequiredValue(settings, CONF_PROCESS_ACCELERATION_MM_S, profile.id).toDouble();

                QString paramsType = getRequiredValue(settings, CONF_PROCESS_PARAMS_TYPE, profile.id).toString();

                if (paramsType == CONF_PROCESS_PARAMS_TYPE_STEPPER)
                {
                    profile.params = stepper_kinematics_params_t{
                        .stepFraction = (uint8_t)getRequiredValue(settings, CONF_PROCESS_STEP_FRACTION, profile.id).toUInt()};
                }
                else if (paramsType != CONF_PROCESS_PARAMS_TYPE_GENERIC)
                {
                    qWarning().nospace() << "WARNING: Parameters type '" << paramsType << "' (profile: " << motorId << "/" << profile.id << ") not handled";
                }

                config.kinematic_profiles[motorId][profile.id] = profile;
                settings.endGroup(); // profileGroup
            }
            settings.endGroup(); // motorGroup
        }
        settings.endGroup(); // CONF_PROCESS_KINEMATICS

        const std::string cameraGroup(CONF_PROCESS_CAMERAS);
        const std::string alignmentGroup(CONF_PROCESS_ALIGNMENT_POSITIONS);
        const std::string drawersGroup(CONF_PROCESS_DRAWERS_POSITIONS);
        const std::string forceLimitsGroup(CONF_PROCESS_FORCE_LIMITS);
        const std::string admittanceGroup(CONF_PROCESS_ADMITTANCE_TUNING);
        const std::string zElevatorGroup(CONF_PROCESS_ELEVATOR_POSITIONS);

        // =============================
        // LOAD CAMERAS DATA
        // =============================
        settings.beginGroup(cameraGroup);
        config.vision.min_camera_distance_mm   = getRequiredValue(settings, CONF_PROCESS_MIN_CAMERA_DISTANCE_MM, cameraGroup).toDouble();
        config.vision.left_cam_x_reset_pos_mm  = getRequiredValue(settings, CONF_PROCESS_LEFT_CAM_X_RESET_POS_MM, cameraGroup).toDouble();
        config.vision.left_cam_y_reset_pos_mm  = getRequiredValue(settings, CONF_PROCESS_LEFT_CAM_Y_RESET_POS_MM, cameraGroup).toDouble();
        config.vision.right_cam_x_reset_pos_mm = getRequiredValue(settings, CONF_PROCESS_RIGHT_CAM_X_RESET_POS_MM, cameraGroup).toDouble();
        config.vision.right_cam_y_reset_pos_mm = getRequiredValue(settings, CONF_PROCESS_RIGHT_CAM_Y_RESET_POS_MM, cameraGroup).toDouble();
        settings.endGroup(); // CONF_PROCESS_CAMERAS

        // =============================
        // SAVE ALIGNMENT POSITIONS
        // =============================
        settings.beginGroup(alignmentGroup);
        config.alignment.x_stage_center_pos_mm     = getRequiredValue(settings, CONF_PROCESS_X_STAGE_CENTER_POS_MM, alignmentGroup).toDouble();
        config.alignment.y_stage_center_pos_mm     = getRequiredValue(settings, CONF_PROCESS_Y_STAGE_CENTER_POS_MM, alignmentGroup).toDouble();
        config.alignment.theta_stage_center_pos_mm = getRequiredValue(settings, CONF_PROCESS_THETA_STAGE_CENTER_POS_MM, alignmentGroup).toDouble();
        settings.endGroup();

        settings.beginGroup(zElevatorGroup);
        config.elevator.max_z_relative_distance_mm = getRequiredValue(settings, CONF_PROCESS_MAX_Z_RELATIVE_DISTANCE_MM, zElevatorGroup).toDouble();
        settings.endGroup();

        // =============================
        // SAVE DRAWERS POSITIONS
        // =============================
        settings.beginGroup(drawersGroup);
        config.drawers.cm3_reset_pos_mm = getRequiredValue(settings, CONF_PROCESS_CM3_RESET_POS_MM, drawersGroup).toDouble();
        settings.endGroup();

        // =============================
        // FORCE LIMITS
        // =============================
        settings.beginGroup(forceLimitsGroup);
        config.contact.hw_crash_force_limit_gf      = getRequiredValue(settings, CONF_PROCESS_HW_CRASH_FORCE_LIMIT_GF, forceLimitsGroup).toDouble();
        config.contact.max_process_force_gf         = getRequiredValue(settings, CONF_PROCESS_MAX_FORCE_GF, forceLimitsGroup).toDouble();
        config.contact.contact_threshold_gf         = getRequiredValue(settings, CONF_PROCESS_CONTACT_THRESHOLD_GF, forceLimitsGroup).toDouble();
        config.contact.autolevel_force_gf           = getRequiredValue(settings, CONF_PROCESS_AUTOLEVEL_FORCE_GF, forceLimitsGroup).toDouble();
        config.contact.autolevel_force_tolerance_gf = getRequiredValue(settings, CONF_PROCESS_AUTOLEVEL_FORCE_TOLERANCE_GF, forceLimitsGroup).toDouble();
        settings.endGroup(); // forceLimitsGroup

        // =============================
        // ADMITTANCE TUNING VALUES
        // =============================
        settings.beginGroup(admittanceGroup);
        config.contact.admittance = {
            .max_step_mm_per_tick          = getRequiredValue(settings, CONF_PROCESS_ADMITTANCE_MAX_STEP_MM_PER_TICK, admittanceGroup).toDouble(),
            .translational_gain_low_force  = getRequiredValue(settings, CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_LOW_FORCE, admittanceGroup).toDouble(),
            .translational_gain_high_force = getRequiredValue(settings, CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_HIGH_FORCE, admittanceGroup).toDouble(),
            .rotational_gain_low_force     = getRequiredValue(settings, CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_LOW_FORCE, admittanceGroup).toDouble(),
            .rotational_gain_high_force    = getRequiredValue(settings, CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_HIGH_FORCE, admittanceGroup).toDouble(),
        };
        settings.endGroup(); // admittanceGroup

        return config;
    }

} // namespace Kub3::Config
