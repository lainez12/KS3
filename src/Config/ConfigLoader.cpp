#include <QSettings>
#include <QStringList>
#include <format>
#include <stdexcept>

#include <Config/ConfigLoader.h>
#include <Config/keys/hardware.h>
#include <Config/keys/process.h>

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
        qInfo() << "[LOADING HARDWARE CONFIGURATION]";
        hardware_config_t config;
        QSettings settings(QString::fromStdString(filePath), QSettings::IniFormat);

        if (settings.status() != QSettings::NoError)
        {
            throw std::runtime_error("CRITICAL: Failed to open or parse hardware config file: " + filePath);
        }

        // LOAD MOTORS PARAMETERS
        settings.beginGroup(CONF_MOTORS);
        for (const QString &group : settings.childGroups())
        {
            settings.beginGroup(group);

            motor_config_t motor;
            motor.id = group.toStdString();

            QString type = getRequiredValue(settings, CONF_MOTOR_TYPE, motor.id).toString();

            if (type == CONF_MOTOR_TYPE_STEPPER)
            {
                stepper_hw_properties_t hw;

                // Parse, Don't Validate: We immediately cast to correct type. If it's malformed, it throws.
                hw.stepsPerRev         = getRequiredValue(settings, CONF_MOTOR_STEPS_PER_REV, motor.id).toUInt();
                hw.screwPitchMm        = getRequiredValue(settings, CONF_SCREW_PITCH_MM, motor.id).toDouble();
                hw.maxVelocityMmS      = getRequiredValue(settings, CONF_MAX_VELOCITY_MM_S, motor.id).toDouble();
                hw.maxAccelerationMmS2 = getRequiredValue(settings, CONF_MAX_ACCELERATION_MM_S2, motor.id).toDouble();
                hw.encoderTopsPerRev   = getRequiredValue(settings, CONF_ENCODER_TOPS_PER_REV, motor.id).toUInt();
                // TODO: more checks
                if (hw.screwPitchMm == 0.0)
                    throw std::runtime_error(std::format("CRITICAL: Invalid screw pitch value ({})", hw.screwPitchMm));
                motor.hwProperties = hw;
            }
            else
            {
                throw std::runtime_error(std::format("CRITICAL: Unknown motor type '{}' for '{}'", type.toStdString(), motor.id));
            }

            config.motors.emplace(group, motor);
            settings.endGroup();
        }
        settings.endGroup(); // CONF_MOTORS

        // LOAD CAMERAS PARAMETERS
        settings.beginGroup(CONF_CAMERAS);
        for (const QString &group : settings.childGroups())
        {
            settings.beginGroup(group);

            camera_config_t camera = {
                .id                = group.toStdString(),
                .serialNumber      = getRequiredValue(settings, CONF_SERIAL_NUMBER, group).toString().toStdString(),
                .maxExposureUs     = getRequiredValue(settings, CONF_MAX_EXPOSURE_US, group).toDouble(),
                .defaultExposureUs = getRequiredValue(settings, CONF_DEFAULT_EXPOSURE_US, group).toDouble(),
                .maxGainDb         = getRequiredValue(settings, CONF_MAX_GAIN_DB, group).toDouble(),
                .defaultGainDb     = getRequiredValue(settings, CONF_DEFAULT_GAIN_DB, group).toDouble(),
            };

            config.cameras.emplace(group, camera);
            settings.endGroup(); // group
        }
        settings.endGroup(); // CONF_CAMERAS

        settings.beginGroup(CONF_FORCE_SENSORS);
        for (const QString &group : settings.childGroups())
        {
            settings.beginGroup(group);
            config.adc_to_gf_factors.emplace(group, getRequiredValue(settings, ADC_TO_GRAM_FORCE_FACTOR, group).toDouble());
            settings.endGroup(); // group
        }
        settings.endGroup(); // CONF_FORCE_SENSORS

        return config;
    }

    process_config_t ConfigLoader::loadProcessConfig(const std::string &filePath)
    {
        qInfo() << "[LOADING PROCESS CONFIGURATION]";
        process_config_t config;
        QSettings settings(QString::fromStdString(filePath), QSettings::IniFormat);

        if (settings.status() != QSettings::NoError)
        {
            throw std::runtime_error("CRITICAL: Failed to open or parse process config file: " + filePath);
        }

        // LOAD KINEMATIC PROFILES
        settings.beginGroup("kinematics");
        qInfo() << "--- Loading kinematics config";
        for (const QString &motorGroup : settings.childGroups())
        {
            settings.beginGroup(motorGroup);
            const std::string motorId = motorGroup.toStdString();

            for (const QString &profileGroup : settings.childGroups())
            {
                settings.beginGroup(profileGroup);
                kinematic_profile_t profile;

                profile.id                 = profileGroup.toStdString();
                profile.initialVelocityMmS = getRequiredValue(settings, "initial_velocity_mm_s", profile.id).toDouble();
                profile.targetVelocityMmS  = getRequiredValue(settings, "target_velocity_mm_s", profile.id).toDouble();
                profile.accelerationMmS2   = getRequiredValue(settings, "acceleration_mm_s", profile.id).toDouble();

                qInfo().nospace() << "------ Loading profile " << profile.id << " for motor " << motorId;
                qInfo() << "\t> Initial velocity (mm/s):" << profile.initialVelocityMmS;
                qInfo() << "\t> Target velocity (mm/s):" << profile.targetVelocityMmS;
                qInfo() << "\t> Acceleration (mm/s2):" << profile.accelerationMmS2;

                QString paramsType = getRequiredValue(settings, "params_type", profile.id).toString();

                if (paramsType == "stepper")
                {
                    profile.params = stepper_kinematics_params_t{
                        .stepFraction = (uint8_t)getRequiredValue(settings, "step_fraction", profile.id).toUInt()};
                    qInfo().nospace() << "\t> [Stepper] resolution: 1/" << std::get<stepper_kinematics_params_t>(profile.params).stepFraction;
                }
                else
                {
                    qWarning().nospace() << "WARNING: No parameters type specified for kinematic profile " << motorId << "/" << profile.id;
                }

                config.kinematic_profiles[motorId][profile.id] = profile;
                settings.endGroup(); // profileGroup
            }
            settings.endGroup(); // motorGroup
        }
        settings.endGroup(); // "kinematics"

        // LOAD CAMERAS DATA
        settings.beginGroup("cameras");
        config.min_camera_distance_mm = getRequiredValue(settings, "min_camera_distance_mm", std::string("cameras")).toDouble();
        settings.endGroup(); // "cameras"

        const std::string forceLimitsGroup("forceLimits");
        settings.beginGroup(forceLimitsGroup);
        config.hw_crash_force_limit_gf = getRequiredValue(settings, "hw_crash_force_limit_gf", forceLimitsGroup).toDouble();
        config.max_force_gf            = getRequiredValue(settings, "max_force_gf", forceLimitsGroup).toDouble();
        config.contact_threshold_gf    = getRequiredValue(settings, "contact_threshold_gf", forceLimitsGroup).toDouble();
        settings.endGroup(); // forceLimitsGroup

        return config;
    }

} // namespace Kub3::Config
