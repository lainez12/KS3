#pragma once

#include <format>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <Config/default/process.h>
#include <Config/kinematics.h>

namespace Kub3::Config
{

    ///////////////////////////
    // PROCESS CONFIGURATION
    ///////////////////////////

    using KinematicProfiles = std::unordered_map<std::string, kinematic_profile_t>;

    // ------------------------------------------
    // Z-Axis Admittance / WEC Tuning
    // ------------------------------------------
    typedef struct admittance_tuning_config_s {
        double max_step_mm_per_tick   = CONF_PROCESS_ADMITTANCE_MAX_STEP_MM_PER_TICK_DEFAULT;   // Safety limit: max blind distance per 20ms evaluation
        double deadband_velocity_mm_s = CONF_PROCESS_ADMITTANCE_DEADBAND_VELOCITY_MM_S_DEFAULT; // Minimum velocity (deadband entered)

        // Translational (Elevator) Gains
        double translational_gain_low_force  = CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_LOW_FORCE_DEFAULT;  // Mapped to k_mean_max (mm/s/gf) - Fast approach
        double translational_gain_high_force = CONF_PROCESS_ADMITTANCE_TRANSLATION_GAIN_HIGH_FORCE_DEFAULT; // Mapped to k_mean_min (mm/s/gf) - Fine pressing

        // Rotational (Tilt/Suspension) Gains
        double rotational_gain_low_force  = CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_LOW_FORCE_DEFAULT;  // Mapped to k_tilt_max (mm/s/gf) - Fast planarization
        double rotational_gain_high_force = CONF_PROCESS_ADMITTANCE_ROTATION_GAIN_HIGH_FORCE_DEFAULT; // Mapped to k_tilt_min (mm/s/gf) - Fine planarization
    } admittance_tuning_config_t;

    // ------------------------------------------
    // Sub-System Process Configurations
    // ------------------------------------------

    typedef struct contact_process_config_s {
        // Process Targets
        double contact_threshold_gf         = CONF_PROCESS_CONTACT_THRESHOLD_GF_DEFAULT; // Threshold exceeded when contact is achieved (gram-force)
        double autolevel_force_gf           = CONF_PROCESS_AUTOLEVEL_FORCE_GF_DEFAULT;   // Force to autolevel (gram-force)
        double autolevel_force_tolerance_gf = CONF_PROCESS_AUTOLEVEL_FORCE_TOLERANCE_GF_DEFAULT;
        double max_process_force_gf         = CONF_PROCESS_MAX_FORCE_GF_DEFAULT; // Max force allowed to be requested (gram-force)

        // Safety Limits
        double hw_crash_force_limit_gf = CONF_PROCESS_HW_CRASH_FORCE_LIMIT_GF_DEFAULT; // Absolute critical limit (gram-force)

        // Control Loop Tuning
        admittance_tuning_config_t admittance;
    } contact_process_config_t;

    typedef struct elevator_process_config_s {
        // Safety limits
        double max_z_relative_distance_mm = CONF_PROCESS_MAX_Z_RELATIVE_DISTANCE_MM_DEFAULT;
    } elevator_process_config_t;

    typedef struct focal_configuration_s {
        uint32_t default_value = CONF_PROCESS_FOCAL_DEFAULT_VALUE_DEFAULT;
        uint32_t min_value     = CONF_PROCESS_FOCAL_MIN_VALUE_DEFAULT;
        uint32_t max_value     = CONF_PROCESS_FOCAL_MAX_VALUE_DEFAULT;
    } focal_conf_t;

    typedef struct vision_process_config_s {
        // Safety limits
        double min_camera_distance_mm = CONF_PROCESS_MIN_CAMERA_DISTANCE_MM_DEFAULT;
        // Reset positions
        double left_cam_x_reset_pos_mm  = CONF_PROCESS_LEFT_CAM_X_RESET_POS_MM_DEFAULT;
        double left_cam_y_reset_pos_mm  = CONF_PROCESS_LEFT_CAM_Y_RESET_POS_MM_DEFAULT;
        double right_cam_x_reset_pos_mm = CONF_PROCESS_RIGHT_CAM_X_RESET_POS_MM_DEFAULT;
        double right_cam_y_reset_pos_mm = CONF_PROCESS_RIGHT_CAM_Y_RESET_POS_MM_DEFAULT;
        // Home positions
        double left_cam_x_home_pos_mm  = CONF_PROCESS_LEFT_CAM_X_HOME_POS_MM_DEFAULT;
        double left_cam_y_home_pos_mm  = CONF_PROCESS_LEFT_CAM_Y_HOME_POS_MM_DEFAULT;
        double right_cam_x_home_pos_mm = CONF_PROCESS_RIGHT_CAM_X_HOME_POS_MM_DEFAULT;
        double right_cam_y_home_pos_mm = CONF_PROCESS_RIGHT_CAM_Y_HOME_POS_MM_DEFAULT;
        // Virtual limits
        double left_cam_x_virtual_limit_mm  = CONF_PROCESS_LEFT_CAM_X_VIRTUAL_LIMIT_POS_MM_DEFAULT;
        double right_cam_x_virtual_limit_mm = CONF_PROCESS_RIGHT_CAM_X_VIRTUAL_LIMIT_POS_MM_DEFAULT;
        // Focals
        focal_conf_t left_focal_conf;
        focal_conf_t right_focal_conf;
    } vision_process_config_t;

    typedef struct pad_process_config_s {
        // Alignment / Z-axis
        double x_stage_distance_mm     = CONF_PROCESS_X_STAGE_DISTANCE_MM_DEFAULT;
        double y_stage_distance_mm     = CONF_PROCESS_Y_STAGE_DISTANCE_MM_DEFAULT;
        double theta_stage_distance_mm = CONF_PROCESS_THETA_STAGE_DISTANCE_MM_DEFAULT;
        double z_motors_distance_mm    = CONF_PROCESS_Z_MOTORS_DISTANCE_MM_DEFAULT;
        // Cameras movements
        double left_cam_x_distance_mm  = CONF_PROCESS_LEFT_CAM_X_DISTANCE_MM_DEFAULT;
        double right_cam_x_distance_mm = CONF_PROCESS_RIGHT_CAM_X_DISTANCE_MM_DEFAULT;
        double left_cam_y_distance_mm  = CONF_PROCESS_LEFT_CAM_Y_DISTANCE_MM_DEFAULT;
        double right_cam_y_distance_mm = CONF_PROCESS_RIGHT_CAM_Y_DISTANCE_MM_DEFAULT;
        // Cameras lighting and focus
        double left_cam_light_step_pc  = CONF_PROCESS_LEFT_CAM_LIGHTING_STEP_PC_DEFAULT;
        double right_cam_light_step_pc = CONF_PROCESS_RIGHT_CAM_LIGHTING_STEP_PC_DEFAULT;
        double left_cam_focal_step_pc  = CONF_PROCESS_LEFT_CAM_FOCAL_STEP_PC_DEFAULT;
        double right_cam_focal_step_pc = CONF_PROCESS_RIGHT_CAM_FOCAL_STEP_PC_DEFAULT;
    } pad_process_config_t;

    typedef struct alignment_process_config_s {
        // Centering positions
        double x_stage_center_pos_mm     = CONF_PROCESS_X_STAGE_CENTER_POS_MM_DEFAULT;
        double y_stage_center_pos_mm     = CONF_PROCESS_Y_STAGE_CENTER_POS_MM_DEFAULT;
        double theta_stage_center_pos_mm = CONF_PROCESS_THETA_STAGE_CENTER_POS_MM_DEFAULT;
    } alignment_process_config_t;

    typedef struct drawer_process_config_s {
        // Reset positions
        double cm3_reset_pos_mm              = CONF_PROCESS_CM3_RESET_POS_MM_DEFAULT;
        double mask_conv_eject_decel_pos_mm  = CONF_PROCESS_MASK_CONV_EJECT_DECEL_POS_MM_DEFAULT;
        double wafer_conv_eject_decel_pos_mm = CONF_PROCESS_WAFER_CONV_EJECT_DECEL_POS_MM_DEFAULT;
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
        pad_process_config_t pad;

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

        void mergeMissingFieldsFrom(const process_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            for (const auto &[motorId, source_profiles] : source.kinematic_profiles)
            {
                auto motorIt           = kinematic_profiles.find(motorId);
                std::string motor_path = std::format("{}/kinematic_profiles[{}]", path, motorId);

                if (motorIt == kinematic_profiles.end())
                {
                    kinematic_profiles[motorId] = source_profiles;
                    logs.push_back(std::format("{} = <created from defaults>", motor_path));
                }
                else
                {
                    for (const auto &[profileName, source_profile] : source_profiles)
                    {
                        std::string profile_path = std::format("{}/{}", motor_path, profileName);
                        if (motorIt->second.find(profileName) == motorIt->second.end())
                        {
                            motorIt->second[profileName] = source_profile;
                            logs.push_back(std::format("{} = <created from defaults>", profile_path));
                        }
                    }
                }
            }
        }
    } process_config_t;

} // namespace Kub3::Config
