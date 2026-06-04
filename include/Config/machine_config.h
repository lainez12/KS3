#pragma once

#include <QHash>
#include <QString>
#include <array>
#include <format>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include "kinematics.h"

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
    } camera_config_t;

    // Top level struct for hardware config
    typedef struct hardware_config_s {
        mcu_hw_properties_t mcus[MCU_COUNT];
        std::unordered_map<QString, motor_config_t> motors;
        std::unordered_map<QString, camera_config_t> cameras;
        std::unordered_map<QString, double> adc_to_gf_factors;
    } hardware_config_t;

    ///////////////////////////
    // SOFTWARE CONFIGURATION
    ///////////////////////////

    using KinematicProfiles = std::unordered_map<std::string, kinematic_profile_t>;

    // ------------------------------------------
    // Z-Axis Admittance / WEC Tuning
    // ------------------------------------------
    typedef struct admittance_tuning_config_s {
        double max_step_mm_per_tick; // Safety limit: max blind distance per 20ms evaluation

        // Translational (Elevator) Gains
        double translational_gain_low_force;  // Mapped to k_mean_max (mm/s/gf) - Fast approach
        double translational_gain_high_force; // Mapped to k_mean_min (mm/s/gf) - Fine pressing

        // Rotational (Tilt/Suspension) Gains
        double rotational_gain_low_force;  // Mapped to k_tilt_max (mm/s/gf) - Fast planarization
        double rotational_gain_high_force; // Mapped to k_tilt_min (mm/s/gf) - Fine planarization
    } admittance_tuning_config_t;

    // ------------------------------------------
    // Sub-System Process Configurations
    // ------------------------------------------

    typedef struct contact_process_config_s {
        // Safety Limits
        double hw_crash_force_limit_gf;

        // Process Targets
        double contact_threshold_gf;
        double autolevel_force_gf;
        double max_process_force_gf;

        // Control Loop Tuning
        admittance_tuning_config_t admittance;
    } contact_process_config_t;

    typedef struct elevator_process_config_s {
        // Safety limits
        double max_z_relative_distance_mm;
    } elevator_process_config_t;

    typedef struct vision_process_config_s {
        // Safety limits
        double min_camera_distance_mm = 0.0;

        // Reset positions
        double left_cam_x_reset_pos_mm  = 0.0;
        double left_cam_y_reset_pos_mm  = 0.0;
        double right_cam_x_reset_pos_mm = 0.0;
        double right_cam_y_reset_pos_mm = 0.0;
    } vision_process_config_t;

    typedef struct alignment_process_config_s {
        // Centering positions
        double x_stage_center_pos_mm     = 0.0;
        double y_stage_center_pos_mm     = 0.0;
        double theta_stage_center_pos_mm = 0.0;
    } alignment_process_config_t;

    typedef struct drawer_process_config_s {
        // Reset positions
        double cm3_reset_pos_mm = 0.0;
    } drawer_process_config_t;

    // ------------------------------------------
    // Root Process Configuration
    // ------------------------------------------

    // Top level struct for software/process config
    typedef struct process_config_s {
        // Kinematics Dictionary
        std::unordered_map<std::string, KinematicProfiles> kinematic_profiles;

        // Domain-Specific Configurations
        drawer_process_config_t drawers;
        elevator_process_config_t elevator;
        alignment_process_config_t alignment;
        contact_process_config_t contact;
        vision_process_config_t vision;

        [[nodiscard]] kinematic_profile_t getKinematicProfile(const std::string &motorId, const std::string &profileName) const
        {
            auto motorIt = kinematic_profiles.find(motorId);
            if (motorIt == kinematic_profiles.end())
                throw std::runtime_error(std::format("CRITICAL: No kinematic profiles found for motor '{}'", motorId));

            auto profileIt = motorIt->second.find(profileName);
            if (profileIt == motorIt->second.end())
                throw std::runtime_error(std::format("CRITICAL: Profile '{}' not found for motor '{}'", profileName, motorId));

            return profileIt->second;
        }
    } process_config_t;
} // namespace Kub3::Config
