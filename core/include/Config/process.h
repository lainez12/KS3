#pragma once

#include <array>
#include <format>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

#include <Config/kinematics.h>
#include <Config/utils.h>

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
        double max_step_mm_per_tick;   // Safety limit: max blind distance per 20ms evaluation
        double deadband_velocity_mm_s; // Minimum velocity (deadband entered)

        // Translational (Elevator) Gains
        double translational_gain_low_force;  // Mapped to k_mean_max (mm/s/gf) - Fast approach
        double translational_gain_high_force; // Mapped to k_mean_min (mm/s/gf) - Fine pressing

        // Rotational (Tilt/Suspension) Gains
        double rotational_gain_low_force;  // Mapped to k_tilt_max (mm/s/gf) - Fast planarization
        double rotational_gain_high_force; // Mapped to k_tilt_min (mm/s/gf) - Fine planarization

        void mergeMissingFieldsFrom(const admittance_tuning_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0.0, max_step_mm_per_tick, path, logs);
            MERGE_FIELD(*this, source, 0.0, deadband_velocity_mm_s, path, logs);
            MERGE_FIELD(*this, source, 0.0, translational_gain_low_force, path, logs);
            MERGE_FIELD(*this, source, 0.0, translational_gain_high_force, path, logs);
            MERGE_FIELD(*this, source, 0.0, rotational_gain_low_force, path, logs);
            MERGE_FIELD(*this, source, 0.0, rotational_gain_high_force, path, logs);
        }
    } admittance_tuning_config_t;

    // ------------------------------------------
    // Sub-System Process Configurations
    // ------------------------------------------

    typedef struct contact_process_config_s {
        // Process Targets
        double contact_threshold_gf; // Threshold exceeded when contact is achieved (gram-force)
        double autolevel_force_gf;   // Force to be reached by each sensor when performing autoleveling (grap-force)
        double autolevel_force_tolerance_gf;
        double max_process_force_gf; // Max force allowed to be requested (gram-force)

        // Safety Limits
        double hw_crash_force_limit_gf; // Absolute critical limit not to be exceeded (gram-force)

        // Control Loop Tuning
        admittance_tuning_config_t admittance;

        void mergeMissingFieldsFrom(const contact_process_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0.0, contact_threshold_gf, path, logs);
            MERGE_FIELD(*this, source, 0.0, autolevel_force_gf, path, logs);
            MERGE_FIELD(*this, source, 0.0, autolevel_force_tolerance_gf, path, logs);
            MERGE_FIELD(*this, source, 0.0, max_process_force_gf, path, logs);
            MERGE_FIELD(*this, source, 0.0, hw_crash_force_limit_gf, path, logs);

            admittance.mergeMissingFieldsFrom(source.admittance, std::format("{}/admittance", path), logs);
        }
    } contact_process_config_t;

    typedef struct elevator_process_config_s {
        // Safety limits
        double max_z_relative_distance_mm;

        void mergeMissingFieldsFrom(const elevator_process_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0.0, max_z_relative_distance_mm, path, logs);
        }
    } elevator_process_config_t;

    typedef struct focal_configuration_s {
        uint32_t default_value = 0;
        uint32_t min_value     = 0;
        uint32_t max_value     = 4095;

        void mergeMissingFieldsFrom(const focal_configuration_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0, default_value, path, logs);
            MERGE_FIELD(*this, source, 0, min_value, path, logs);
            MERGE_FIELD(*this, source, 4095, max_value, path, logs);
        }
    } focal_conf_t;

    typedef struct vision_process_config_s {
        // Safety limits
        double min_camera_distance_mm = 5.0;
        // Reset positions
        double left_cam_x_reset_pos_mm  = 0.0;
        double left_cam_y_reset_pos_mm  = 0.0;
        double right_cam_x_reset_pos_mm = 0.0;
        double right_cam_y_reset_pos_mm = 0.0;
        // Home positions
        double left_cam_x_home_pos_mm  = 0.0;
        double left_cam_y_home_pos_mm  = 0.0;
        double right_cam_x_home_pos_mm = 0.0;
        double right_cam_y_home_pos_mm = 0.0;
        // Focals
        focal_conf_t left_focal_conf;
        focal_conf_t right_focal_conf;

        void mergeMissingFieldsFrom(const vision_process_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0.0, min_camera_distance_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, left_cam_x_reset_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, left_cam_y_reset_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, right_cam_x_reset_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, right_cam_y_reset_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, left_cam_x_home_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, left_cam_y_home_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, right_cam_x_home_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, right_cam_y_home_pos_mm, path, logs);

            left_focal_conf.mergeMissingFieldsFrom(source.left_focal_conf, std::format("{}/left_focal_conf", path), logs);
            right_focal_conf.mergeMissingFieldsFrom(source.right_focal_conf, std::format("{}/right_focal_conf", path), logs);
        }
    } vision_process_config_t;

    typedef struct pad_process_config_s {
        double left_cam_x_distance_mm  = 0.0;
        double right_cam_x_distance_mm = 0.0;
        double left_cam_y_distance_mm  = 0.0;
        double right_cam_y_distance_mm = 0.0;
        double x_stage_distance_mm     = 0.0;
        double y_stage_distance_mm     = 0.0;
        double theta_stage_distance_mm = 0.0;
        double z_motors_distance_mm    = 0.0;

        void mergeMissingFieldsFrom(const pad_process_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0.0, left_cam_x_distance_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, right_cam_x_distance_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, left_cam_y_distance_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, right_cam_y_distance_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, x_stage_distance_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, y_stage_distance_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, theta_stage_distance_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, z_motors_distance_mm, path, logs);
        }
    } pad_process_config_t;

    typedef struct alignment_process_config_s {
        // Centering positions
        double x_stage_center_pos_mm     = 0.0;
        double y_stage_center_pos_mm     = 0.0;
        double theta_stage_center_pos_mm = 0.0;

        void mergeMissingFieldsFrom(const alignment_process_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0.0, x_stage_center_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, y_stage_center_pos_mm, path, logs);
            MERGE_FIELD(*this, source, 0.0, theta_stage_center_pos_mm, path, logs);
        }
    } alignment_process_config_t;

    typedef struct drawer_process_config_s {
        // Reset positions
        double cm3_reset_pos_mm = 0.0;

        void mergeMissingFieldsFrom(const drawer_process_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, 0.0, cm3_reset_pos_mm, path, logs);
        }
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

            drawers.mergeMissingFieldsFrom(source.drawers, std::format("{}/drawers", path), logs);
            elevator.mergeMissingFieldsFrom(source.elevator, std::format("{}/elevator", path), logs);
            alignment.mergeMissingFieldsFrom(source.alignment, std::format("{}/alignment", path), logs);
            contact.mergeMissingFieldsFrom(source.contact, std::format("{}/contact", path), logs);
            vision.mergeMissingFieldsFrom(source.vision, std::format("{}/vision", path), logs);
            pad.mergeMissingFieldsFrom(source.pad, std::format("{}/pad", path), logs);
        }
    } process_config_t;

} // namespace Kub3::Config
