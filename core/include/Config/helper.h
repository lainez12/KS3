#pragma once

#include <QDebug>
#include <QString>
#include <QStringList>
#include <type_traits>
#include <variant>

#include "conf.h"

namespace Kub3::Config
{
    // Helper function to handle indentation spacing
    inline QString getIndent(int level)
    {
        return QString(level * 4, ' '); // 4 spaces per indent level
    }

    // --- Stepper Kinematic Parameters Formatter ---
    inline QString toString(const stepper_kinematics_params_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1- Step Fraction: %2")
            .arg(ind)
            // Cast uint8_t to int so it formats as a number instead of an ASCII character
            .arg(static_cast<int>(config.stepFraction));
    }

    // --- Motor Kinematics Variant Formatter ---
    inline QString toString(const MotorKinematicsParams &config, int indent = 0)
    {
        return std::visit([indent](auto &&arg) -> QString {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>)
            {
                return QString("%1- Kinematics Params: None").arg(getIndent(indent));
            }
            else if constexpr (std::is_same_v<T, stepper_kinematics_params_t>)
            {
                QString ind = getIndent(indent);
                return QString("%1Stepper Kinematics Params:\n%2")
                    .arg(ind)
                    .arg(toString(arg, indent + 1));
            }
            else
            {
                return QString("%1- Unknown Kinematics Params").arg(getIndent(indent));
            }
        },
                          config);
    }

    // --- Kinematic Profile Formatter ---
    inline QString toString(const kinematic_profile_t &profile, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1- Profile ID: %2\n"
                       "%1- Target Velocity: %4 mm/s\n"
                       "%1- Acceleration: %5 mm/s²\n"
                       "%6")
            .arg(ind)
            .arg(QString::fromStdString(profile.id))
            .arg(profile.targetVelocityMmS)
            .arg(profile.accelerationMmS2)
            .arg(toString(profile.params, indent));
    }

    // --- Hardware Sub-structures ---

    inline QString toString(const mcu_hw_properties_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1- Port: %2\n"
                       "%1- Baudrate: %3")
            .arg(ind)
            .arg(config.port.isEmpty() ? "N/A" : config.port)
            .arg(config.baudrate);
    }

    inline QString toString(const stepper_hw_properties_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1Stepper Properties:\n"
                       "%1  - Steps per Rev: %2\n"
                       "%1  - Screw Pitch: %3 mm\n"
                       "%1  - Max Velocity: %4 mm/s\n"
                       "%1  - Max Acceleration: %5 mm/s²\n"
                       "%1  - Encoder Tops per Rev: %6")
            .arg(ind)
            .arg(config.stepsPerRev)
            .arg(config.screwPitchMm)
            .arg(config.maxVelocityMmS)
            .arg(config.maxAccelerationMmS2)
            .arg(config.encoderTopsPerRev);
    }

    inline QString toString(const dc_motor_hw_properties_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1DC Motor Properties:\n"
                       "%1  - Screw Pitch: %2 mm\n"
                       "%1  - Max Velocity: %3 mm/s\n"
                       "%1  - Max Acceleration: %4 mm/s²")
            .arg(ind)
            .arg(config.screwPitchMm)
            .arg(config.maxVelocityMmS)
            .arg(config.maxAccelerationMmS2);
    }

    inline QString toString(const motor_hw_properties_t &config, int indent = 0)
    {
        return std::visit([indent](auto &&arg) -> QString {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>)
            {
                return QString("%1- Motor Type: Unconfigured / None").arg(getIndent(indent));
            }
            else
            {
                return toString(arg, indent);
            }
        },
                          config);
    }

    inline QString toString(const motor_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1- Motor ID: %2\n%3")
            .arg(ind)
            .arg(QString::fromStdString(config.id))
            .arg(toString(config.hwProperties, indent + 1));
    }

    inline QString toString(const camera_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1Camera ID: %2\n"
                       "%1  - Display Name: %3\n"
                       "%1  - Serial: %4\n"
                       "%1  - Associated Focal ID: %5\n"
                       "%1  - Associated Light ID: %6\n"
                       "%1  - Max Exposure: %7 us\n"
                       "%1  - Default Exposure: %8 us\n"
                       "%1  - Max Gain: %9 dB\n"
                       "%1  - Default Gain: %10 dB")
            .arg(ind)
            .arg(QString::fromStdString(config.id))
            .arg(QString::fromStdString(config.serialNumber))
            .arg(config.associatedFocalId.value_or("None (std::nullopt)"))
            .arg(config.associatedLightId.value_or("None (std::nullopt)"))
            .arg(config.maxExposureUs)
            .arg(config.defaultExposureUs)
            .arg(config.maxGainDb)
            .arg(config.defaultGainDb);
    }

    inline QString toString(const hardware_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        QStringList sections;

        sections << QString("================ HARDWARE CONFIGURATION ================");

        // MCUs
        QStringList mcuLines;
        mcuLines << QString("%1MCUs:").arg(ind);
        for (int i = 0; i < MCU_COUNT; ++i)
        {
            mcuLines << QString("%1  - MCU [%2]:\n%3")
                            .arg(ind)
                            .arg(i)
                            .arg(toString(config.mcus[i], indent + 2));
        }
        sections << mcuLines.join('\n');

        // Motors
        QStringList motorLines;
        motorLines << QString("%1Motors:").arg(ind);
        if (config.motors.empty())
        {
            motorLines << QString("%1  - None").arg(ind);
        }
        else
        {
            for (auto it = config.motors.begin(); it != config.motors.end(); ++it)
            {
                motorLines << QString("%1  - Key [%2]:\n%3")
                                  .arg(ind)
                                  .arg(it->first)
                                  .arg(toString(it->second, indent + 1));
            }
        }
        sections << motorLines.join('\n');

        // Cameras
        QStringList cameraLines;
        cameraLines << QString("%1Cameras:").arg(ind);
        if (config.cameras.empty())
        {
            cameraLines << QString("%1  - None").arg(ind);
        }
        else
        {
            for (auto it = config.cameras.begin(); it != config.cameras.end(); ++it)
            {
                cameraLines << QString("%1  - Key [%2]:\n%3")
                                   .arg(ind)
                                   .arg(it->first)
                                   .arg(toString(it->second, indent + 2));
            }
        }
        sections << cameraLines.join('\n');

        // ADC to GF factors
        QStringList adcLines;
        adcLines << QString("%1ADC to GF Factors:").arg(ind);
        if (config.adc_to_gf_factors.empty())
        {
            adcLines << QString("%1  - None").arg(ind);
        }
        else
        {
            for (auto it = config.adc_to_gf_factors.begin(); it != config.adc_to_gf_factors.end(); ++it)
            {
                adcLines << QString("%1  - %2: %3 gf/ADC")
                                .arg(ind)
                                .arg(it->first)
                                .arg(it->second);
            }
        }
        sections << adcLines.join('\n');

        sections << QString("%1========================================================").arg(ind);

        // Join sections with double newlines for separation
        return sections.join("\n\n");
    }

    // --- Software Sub-structures ---

    inline QString toString(const admittance_tuning_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1Admittance Tuning:\n"
                       "%1  - Max Distance per Step/Tick: %2 mm\n"
                       "%1  - Deadband velocity: %7 mm/s\n"
                       "%1  - Translational Gain (Low Force): %3 mm/s/gf\n"
                       "%1  - Translational Gain (High Force): %4 mm/s/gf\n"
                       "%1  - Rotational Gain (Low Force): %5 mm/s/gf\n"
                       "%1  - Rotational Gain (High Force): %6 mm/s/gf")
            .arg(ind)
            .arg(config.max_step_mm_per_tick)
            .arg(config.translational_gain_low_force)
            .arg(config.translational_gain_high_force)
            .arg(config.rotational_gain_low_force)
            .arg(config.rotational_gain_high_force)
            .arg(config.deadband_velocity_mm_s);
    }

    inline QString toString(const contact_process_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1Contact Process:\n"
                       "%1  - HW Crash Force Limit: %2 gf\n"
                       "%1  - Contact Threshold: %3 gf\n"
                       "%1  - Autolevel Force: %4 gf\n"
                       "%1  - Autolevel Force Tolerance: %5 gf\n"
                       "%1  - Max Process Force: %6 gf\n\n" // Spacing before sub-structures
                       "%7")
            .arg(ind)
            .arg(config.hw_crash_force_limit_gf)
            .arg(config.contact_threshold_gf)
            .arg(config.autolevel_force_gf)
            .arg(config.autolevel_force_tolerance_gf)
            .arg(config.max_process_force_gf)
            .arg(toString(config.admittance, indent + 1));
    }

    inline QString toString(const elevator_process_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1Elevator Process:\n"
                       "%1  - Max Z Relative Distance: %2 mm")
            .arg(ind)
            .arg(config.max_z_relative_distance_mm);
    }

    inline QString toString(const vision_process_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1Vision Process:\n"
                       "%1  - Min Camera Distance: %2 mm\n"
                       "%1  - Left Cam X Reset Position: %3 mm\n"
                       "%1  - Left Cam Y Reset Position: %4 mm\n"
                       "%1  - Right Cam X Reset Position: %5 mm\n"
                       "%1  - Right Cam Y Reset Position: %6 mm\n"
                       "%1  - Left Cam X Home Position: %7 mm\n"
                       "%1  - Left Cam Y Home Position: %8 mm\n"
                       "%1  - Right Cam X Home Position: %9 mm\n"
                       "%1  - Right Cam Y Home Position: %10 mm\n")
            .arg(ind)
            .arg(config.min_camera_distance_mm)
            .arg(config.left_cam_x_reset_pos_mm)
            .arg(config.left_cam_y_reset_pos_mm)
            .arg(config.right_cam_x_reset_pos_mm)
            .arg(config.right_cam_y_reset_pos_mm)
            .arg(config.left_cam_x_home_pos_mm)
            .arg(config.left_cam_y_home_pos_mm)
            .arg(config.right_cam_x_home_pos_mm)
            .arg(config.right_cam_y_home_pos_mm);
    }

    inline QString toString(const alignment_process_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1Alignment Process:\n"
                       "%1  - X Stage Center Position: %2 mm\n"
                       "%1  - Y Stage Center Position: %3 mm\n"
                       "%1  - Theta Stage Center Position: %4 mm")
            .arg(ind)
            .arg(config.x_stage_center_pos_mm)
            .arg(config.y_stage_center_pos_mm)
            .arg(config.theta_stage_center_pos_mm);
    }

    inline QString toString(const drawer_process_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        return QString("%1Drawer Process:\n"
                       "%1  - CM3 Reset Position: %2 mm")
            .arg(ind)
            .arg(config.cm3_reset_pos_mm);
    }

    inline QString toString(const process_config_t &config, int indent = 0)
    {
        QString ind = getIndent(indent);
        QStringList sections;
        sections << QString("================ PROCESS CONFIGURATION ================");

        // Kinematic Profiles
        QStringList kinLines;
        kinLines << QString("%1Kinematic Profiles Dictionary:").arg(ind);
        if (config.kinematic_profiles.empty())
        {
            kinLines << QString("%1  - None").arg(ind);
        }
        else
        {
            for (auto it = config.kinematic_profiles.begin(); it != config.kinematic_profiles.end(); ++it)
            {
                kinLines << QString("%1  - Motor ID [%2]:")
                                .arg(ind)
                                .arg(QString::fromStdString(it->first));
                for (auto profIt = it->second.begin(); profIt != it->second.end(); ++profIt)
                {
                    kinLines << QString("%1    - Profile [%2]:\n%3")
                                    .arg(ind)
                                    .arg(QString::fromStdString(profIt->first))
                                    .arg(toString(profIt->second, indent + 3));
                }
            }
        }
        sections << kinLines.join('\n');

        // Sub-process sections (each becomes its own section block separated by double newlines)
        sections << toString(config.drawers, indent + 1);
        sections << toString(config.elevator, indent + 1);
        sections << toString(config.alignment, indent + 1);
        sections << toString(config.contact, indent + 1);
        sections << toString(config.vision, indent + 1);

        sections << QString("%1========================================================").arg(ind);

        return sections.join("\n\n");
    }

    // --- QDebug Stream Operators ---

    inline QDebug operator<<(QDebug debug, const hardware_config_t &config)
    {
        QDebugStateSaver saver(debug);
        debug.noquote().nospace() << "\n"
                                  << toString(config, 0);
        return debug;
    }

    inline QDebug operator<<(QDebug debug, const process_config_t &config)
    {
        QDebugStateSaver saver(debug);
        debug.noquote().nospace() << "\n"
                                  << toString(config, 0);
        return debug;
    }
} // namespace Kub3::Config