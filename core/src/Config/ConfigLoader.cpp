#include <QSettings>
#include <QStringList>
#include <format>
#include <stdexcept>
#include <type_traits>

#include <Config/ConfigLoader.h>
#include <Config/keys/admin.h>
#include <Config/keys/hardware.h>
#include <Config/keys/process.h>

#define CHECK_CONFIG_VALUE(cond, elemId, value, valName, strict)                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        if ((strict) && (cond))                                                                                        \
        {                                                                                                              \
            throw std::runtime_error(std::format("CRITICAL: Invalid '{}' value for {} ({})", valName, elemId, value)); \
        }                                                                                                              \
    } while (0);

namespace Kub3::Config
{

    // Type-safe helper to parse configuration fields
    template <typename T, typename Func>
    static void loadField(
        const QSettings &settings,
        const QString &key,
        T &target,
        bool strict,
        std::vector<std::string> &logs,
        Func converter)
    {
        if (settings.contains(key))
        {
            target = converter(settings.value(key));
        }
        else
        {
            if (strict)
            {
                throw std::runtime_error(std::format("CRITICAL: Missing config key '{}' in group '{}'", key.toStdString(), settings.group().toStdString()));
            }

            const auto keyedPath = (settings.group() + "/" + key).toStdString();

            // Key is missing in lenient mode: Target retains its C++ struct default value.
            // Log the default fallback so the generator knows it needs to be written to disk.
            if constexpr (std::is_same_v<T, std::string>)
            {
                logs.push_back(std::format("{} = \"{}\"", keyedPath, target));
            }
            else if constexpr (std::is_same_v<T, QString>)
            {
                logs.push_back(std::format("{} = \"{}\"", keyedPath, target.toStdString()));
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                logs.push_back(std::format("{} = {}", keyedPath, target ? "true" : "false"));
            }
            else
            {
                logs.push_back(std::format("{} = {}", keyedPath, target));
            }
        }
    }

    static double qToDouble(const QVariant &v)
    {
        return v.toDouble();
    }

    static uint qToUint(const QVariant &v)
    {
        return v.toUInt();
    }

    static bool qToBool(const QVariant &v)
    {
        return v.toBool();
    }

    hardware_config_t ConfigLoader::loadHardwareConfig(const std::string &filePath, bool strict, std::vector<std::string> *logs)
    {
        std::vector<std::string> localLogs;
        std::vector<std::string> &activeLogs = logs ? *logs : localLogs;

        hardware_config_t config; // Default-constructed with compile-time defaults
        QSettings settings(QString::fromStdString(filePath), QSettings::IniFormat);

        if (settings.status() != QSettings::NoError)
        {
            if (strict)
                throw std::runtime_error("CRITICAL: Failed to open or parse hardware config file: " + filePath);
        }

        // =============================
        // LOAD LOCALE SETTINGS
        // =============================
        loadField(settings, CONF_HW_KEYBOARD_LAYOUT, config.keyboardLayout, strict, activeLogs, [](const QVariant &v) { return v.toString(); });

        // =============================
        // LOAD MCUs SETTINGS
        // =============================
        settings.beginGroup(CONF_HW_MCUS);
        {
            static_assert(MCU_COUNT == 4, "MCU_COUNT does not match the model expected value (4)");

            loadField(settings, CONF_HW_MCU1_PORT, config.mcus[0].port, strict, activeLogs, [](const QVariant &v) { return v.toString(); });
            loadField(settings, CONF_HW_MCU1_BAUDRATE, config.mcus[0].baudrate, strict, activeLogs, &qToUint);

            loadField(settings, CONF_HW_MCU2_PORT, config.mcus[1].port, strict, activeLogs, [](const QVariant &v) { return v.toString(); });
            loadField(settings, CONF_HW_MCU2_BAUDRATE, config.mcus[1].baudrate, strict, activeLogs, &qToUint);

            loadField(settings, CONF_HW_MCU3_PORT, config.mcus[2].port, strict, activeLogs, [](const QVariant &v) { return v.toString(); });
            loadField(settings, CONF_HW_MCU3_BAUDRATE, config.mcus[2].baudrate, strict, activeLogs, &qToUint);

            loadField(settings, CONF_HW_MCU4_PORT, config.mcus[3].port, strict, activeLogs, [](const QVariant &v) { return v.toString(); });
            loadField(settings, CONF_HW_MCU4_BAUDRATE, config.mcus[3].baudrate, strict, activeLogs, &qToUint);
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

            QString type;
            loadField(settings, CONF_HW_MOTOR_TYPE, type, strict, activeLogs, [](const QVariant &v) { return v.toString(); });

            if (type == CONF_HW_MOTOR_TYPE_STEPPER)
            {
                stepper_hw_properties_t hw;
                loadField(settings, CONF_HW_MOTOR_STEPS_PER_REV, hw.stepsPerRev, strict, activeLogs, &qToUint);
                loadField(settings, CONF_HW_SCREW_PITCH_MM, hw.screwPitchMm, strict, activeLogs, &qToDouble);
                loadField(settings, CONF_HW_MAX_VELOCITY_MM_S, hw.maxVelocityMmS, strict, activeLogs, &qToDouble);
                loadField(settings, CONF_HW_MAX_ACCELERATION_MM_S2, hw.maxAccelerationMmS2, strict, activeLogs, &qToDouble);
                loadField(settings, CONF_HW_ENCODER_TOPS_PER_REV, hw.encoderTopsPerRev, strict, activeLogs, &qToUint);

                CHECK_CONFIG_VALUE(hw.stepsPerRev == 0, motor.id, hw.stepsPerRev, "steps per revolution", strict);
                CHECK_CONFIG_VALUE(hw.screwPitchMm == 0.0, motor.id, hw.screwPitchMm, "screw pitch", strict);
                CHECK_CONFIG_VALUE(hw.encoderTopsPerRev == 0, motor.id, hw.encoderTopsPerRev, "encoder tops per revolution", strict);

                motor.hwProperties = hw;
            }
            else if (type == CONF_HW_MOTOR_TYPE_DC)
            {
                dc_motor_hw_properties_t hw;
                loadField(settings, CONF_HW_SCREW_PITCH_MM, hw.screwPitchMm, strict, activeLogs, &qToDouble);
                loadField(settings, CONF_HW_MAX_VELOCITY_MM_S, hw.maxVelocityMmS, strict, activeLogs, &qToDouble);
                loadField(settings, CONF_HW_MAX_ACCELERATION_MM_S2, hw.maxAccelerationMmS2, strict, activeLogs, &qToDouble);

                CHECK_CONFIG_VALUE(hw.screwPitchMm == 0.0, motor.id, hw.screwPitchMm, "screw pitch", strict);

                motor.hwProperties = hw;
            }
            else if (strict)
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

            camera_config_t camera;
            camera.id = group.toStdString();

            loadField(settings, CONF_HW_SERIAL_NUMBER, camera.serialNumber, strict, activeLogs, [](const QVariant &v) { return v.toString().toStdString(); });
            loadField(settings, CONF_HW_MAX_EXPOSURE_US, camera.maxExposureUs, strict, activeLogs, &qToDouble);
            loadField(settings, CONF_HW_DEFAULT_EXPOSURE_US, camera.defaultExposureUs, strict, activeLogs, &qToDouble);
            loadField(settings, CONF_HW_MAX_GAIN_DB, camera.maxGainDb, strict, activeLogs, &qToDouble);
            loadField(settings, CONF_HW_DEFAULT_GAIN_DB, camera.defaultGainDb, strict, activeLogs, &qToDouble);
            loadField(settings, CONF_HW_FRAMERATE, camera.framerate, strict, activeLogs, &qToDouble);

            if (settings.contains(CONF_HW_ASSOCIATED_FOCAL_ID))
            {
                QString val = settings.value(CONF_HW_ASSOCIATED_FOCAL_ID).toString();
                if (!val.isEmpty())
                    camera.associatedFocalId = val;
            }
            if (settings.contains(CONF_HW_ASSOCIATED_LIGHT_ID))
            {
                QString val = settings.value(CONF_HW_ASSOCIATED_LIGHT_ID).toString();
                if (!val.isEmpty())
                    camera.associatedLightId = val;
            }

            config.cameras.emplace(group, camera);
            settings.endGroup();
        }
        settings.endGroup(); // CONF_HW_CAMERAS

        // =============================
        // FORCE LIMITS
        // =============================
        settings.beginGroup(CONF_HW_FORCE_SENSORS);
        for (const QString &group : settings.childGroups())
        {
            settings.beginGroup(group);
            double val = 1.0;
            loadField(settings, CONF_HW_ADC_TO_GRAM_FORCE_FACTOR, val, strict, activeLogs, &qToDouble);
            config.adc_to_gf_factors.emplace(group, val);
            settings.endGroup();
        }
        settings.endGroup();

        return config;
    }

    process_config_t ConfigLoader::loadProcessConfig(const std::string &filePath, bool strict, std::vector<std::string> *logs)
    {
        std::vector<std::string> localLogs;
        std::vector<std::string> &activeLogs = logs ? *logs : localLogs;

        process_config_t config; // Default constructed: Holds all structural C++ defaults
        QSettings settings(QString::fromStdString(filePath), QSettings::IniFormat);

        if (settings.status() != QSettings::NoError)
        {
            if (strict)
                throw std::runtime_error("CRITICAL: Failed to open or parse process config file: " + filePath);
        }

        // =============================
        // LOAD KINEMATIC PROFILES (Handled recursively)
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
                profile.id = profileGroup.toStdString();

                loadField(settings, CONF_PROCESS_TARGET_VELOCITY_MM_S, profile.targetVelocityMmS, strict, activeLogs, &qToDouble);
                loadField(settings, CONF_PROCESS_ACCELERATION_MM_S, profile.accelerationMmS2, strict, activeLogs, &qToDouble);

                QString paramsType;
                loadField(settings, CONF_PROCESS_PARAMS_TYPE, paramsType, strict, activeLogs, [](const QVariant &v) { return v.toString(); });

                if (paramsType == CONF_PROCESS_PARAMS_TYPE_STEPPER)
                {
                    uint32_t stepFrac = 1;
                    loadField(settings, CONF_PROCESS_STEP_FRACTION, stepFrac, strict, activeLogs, &qToUint);
                    profile.params = stepper_kinematics_params_t{.stepFraction = (uint8_t)stepFrac};
                }
                else if (paramsType != CONF_PROCESS_PARAMS_TYPE_GENERIC && !paramsType.isEmpty())
                {
                    if (strict)
                        qWarning().nospace() << "WARNING: Parameters type '" << paramsType << "' not handled";
                }

                config.kinematic_profiles[motorId][profile.id] = profile;
                settings.endGroup();
            }
            settings.endGroup();
        }
        settings.endGroup();

        // =============================
        // LOAD CAMERAS DATA
        // =============================
        settings.beginGroup(CONF_PROCESS_CAMERAS);
        loadField(settings, CONF_PROCESS_MIN_CAMERA_DISTANCE_MM, config.vision.min_camera_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_LEFT_CAM_X_RESET_POS_MM, config.vision.left_cam_x_reset_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_LEFT_CAM_Y_RESET_POS_MM, config.vision.left_cam_y_reset_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_X_RESET_POS_MM, config.vision.right_cam_x_reset_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_Y_RESET_POS_MM, config.vision.right_cam_y_reset_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_LEFT_CAM_X_HOME_POS_MM, config.vision.left_cam_x_home_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_LEFT_CAM_Y_HOME_POS_MM, config.vision.left_cam_y_home_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_X_HOME_POS_MM, config.vision.right_cam_x_home_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_Y_HOME_POS_MM, config.vision.right_cam_y_home_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_LEFT_CAM_X_VIRTUAL_LIMIT_POS_MM, config.vision.left_cam_x_virtual_limit_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_X_VIRTUAL_LIMIT_POS_MM, config.vision.right_cam_x_virtual_limit_mm, strict, activeLogs, &qToDouble);

        {
            settings.beginGroup(CONF_PROCESS_LEFT_FOCAL);
            loadField(settings, CONF_PROCESS_FOCAL_DEFAULT_VALUE, config.vision.left_focal_conf.default_value, strict, activeLogs, &qToUint);
            loadField(settings, CONF_PROCESS_FOCAL_MIN_VALUE, config.vision.left_focal_conf.min_value, strict, activeLogs, &qToUint);
            loadField(settings, CONF_PROCESS_FOCAL_MAX_VALUE, config.vision.left_focal_conf.max_value, strict, activeLogs, &qToUint);
            settings.endGroup();

            settings.beginGroup(CONF_PROCESS_RIGHT_FOCAL);
            loadField(settings, CONF_PROCESS_FOCAL_DEFAULT_VALUE, config.vision.right_focal_conf.default_value, strict, activeLogs, &qToUint);
            loadField(settings, CONF_PROCESS_FOCAL_MIN_VALUE, config.vision.right_focal_conf.min_value, strict, activeLogs, &qToUint);
            loadField(settings, CONF_PROCESS_FOCAL_MAX_VALUE, config.vision.right_focal_conf.max_value, strict, activeLogs, &qToUint);
            settings.endGroup();
        }
        settings.endGroup();

        // =============================
        // SAVE ALIGNMENT POSITIONS
        // =============================
        settings.beginGroup(CONF_PROCESS_ALIGNMENT_POSITIONS);
        loadField(settings, CONF_PROCESS_X_STAGE_CENTER_POS_MM, config.alignment.x_stage_center_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_Y_STAGE_CENTER_POS_MM, config.alignment.y_stage_center_pos_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_THETA_STAGE_CENTER_POS_MM, config.alignment.theta_stage_center_pos_mm, strict, activeLogs, &qToDouble);
        settings.endGroup();

        settings.beginGroup(CONF_PROCESS_ELEVATOR_POSITIONS);
        loadField(settings, CONF_PROCESS_MAX_Z_RELATIVE_DISTANCE_MM, config.elevator.max_z_relative_distance_mm, strict, activeLogs, &qToDouble);
        settings.endGroup();

        // =============================
        // SAVE DRAWERS POSITIONS
        // =============================
        settings.beginGroup(CONF_PROCESS_DRAWERS_POSITIONS);
        loadField(settings, CONF_PROCESS_CM3_RESET_POS_MM, config.drawers.cm3_reset_pos_mm, strict, activeLogs, &qToDouble);
        settings.endGroup();

        // =============================
        // FORCE LIMITS
        // =============================
        settings.beginGroup(CONF_PROCESS_FORCE_LIMITS);
        loadField(settings, CONF_PROCESS_HW_CRASH_FORCE_LIMIT_GF, config.contact.hw_crash_force_limit_gf, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_MAX_FORCE_GF, config.contact.max_process_force_gf, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_CONTACT_THRESHOLD_GF, config.contact.contact_threshold_gf, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_AUTOLEVEL_FORCE_GF, config.contact.autolevel_force_gf, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_AUTOLEVEL_FORCE_TOLERANCE_GF, config.contact.autolevel_force_tolerance_gf, strict, activeLogs, &qToDouble);
        settings.endGroup();

        // =============================
        // ADMITTANCE TUNING VALUES
        // =============================
        settings.beginGroup(CONF_PROCESS_ADMITTANCE_TUNING);
        loadField(settings, CONF_PROCESS_ADMITTANCE_MAX_STEP_MM_PER_TICK, config.contact.admittance.max_step_mm_per_tick, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_ADMITTANCE_DEADBAND_VELOCITY_MM_S, config.contact.admittance.deadband_velocity_mm_s, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_LOW_FORCE, config.contact.admittance.translational_gain_low_force, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_HIGH_FORCE, config.contact.admittance.translational_gain_high_force, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_LOW_FORCE, config.contact.admittance.rotational_gain_low_force, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_HIGH_FORCE, config.contact.admittance.rotational_gain_high_force, strict, activeLogs, &qToDouble);
        settings.endGroup();

        // =============================
        // PAD MOVEMENTS VALUES
        // =============================
        settings.beginGroup(CONF_PROCESS_PAD_MOVEMENTS);
        settings.group();
        loadField(settings, CONF_PROCESS_LEFT_CAM_X_DISTANCE_MM, config.pad.left_cam_x_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_X_DISTANCE_MM, config.pad.right_cam_x_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_LEFT_CAM_Y_DISTANCE_MM, config.pad.left_cam_y_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_Y_DISTANCE_MM, config.pad.right_cam_y_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_X_STAGE_DISTANCE_MM, config.pad.x_stage_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_Y_STAGE_DISTANCE_MM, config.pad.y_stage_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_THETA_STAGE_DISTANCE_MM, config.pad.theta_stage_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_Z_MOTORS_DISTANCE_MM, config.pad.z_motors_distance_mm, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_LEFT_CAM_LIGHTING_STEP_PC, config.pad.left_cam_light_step_pc, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_LIGHTING_STEP_PC, config.pad.right_cam_light_step_pc, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_LEFT_CAM_FOCAL_STEP_PC, config.pad.left_cam_focal_step_pc, strict, activeLogs, &qToDouble);
        loadField(settings, CONF_PROCESS_RIGHT_CAM_FOCAL_STEP_PC, config.pad.right_cam_focal_step_pc, strict, activeLogs, &qToDouble);
        settings.endGroup();

        return config;
    }

    admin_config_t ConfigLoader::loadAdminConfig(const std::string &filePath, bool strict, std::vector<std::string> *logs)
    {
        std::vector<std::string> localLogs;
        std::vector<std::string> &activeLogs = logs ? *logs : localLogs;

        admin_config_t config;
        QSettings settings(QString::fromStdString(filePath), QSettings::IniFormat);

        if (settings.status() != QSettings::NoError)
        {
            if (strict)
                throw std::runtime_error("CRITICAL: Failed to open or parse admin config file: " + filePath);
        }

        loadField(settings, CONF_ADMIN_KLOE_MODE, config.kloe_mode, strict, activeLogs, &qToBool);

        return config;
    }

} // namespace Kub3::Config
