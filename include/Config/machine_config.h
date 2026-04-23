#pragma once

#include <string>
#include <unordered_map>
#include <variant>

#include "kinematics.h"

namespace Kub3::Config
{

    ///////////////////////////
    // HARDWARE CONFIGURATION
    ///////////////////////////

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
        std::unordered_map<QString, motor_config_t> motors;
        std::unordered_map<QString, camera_config_t> cameras;
    } hardware_config_t;

    ///////////////////////////
    // SOFTWARE CONFIGURATION
    ///////////////////////////

    using KinematicProfiles = std::unordered_map<std::string, kinematic_profile_t>;

    // Top level struct for software/process config
    typedef struct process_config_s {
        // Map [id] -> [map of kinematic profiles for motor]
        std::unordered_map<std::string, KinematicProfiles> kinematic_profiles;

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
