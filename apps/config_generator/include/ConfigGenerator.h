#pragma once

#include <QString>

#include <Config/kinematics.h>

namespace Kub3::Config
{

    class ConfigGenerator
    {
    public:
        /**
         * @brief Checks configuration files and fills any missing mandatory fields with default values.
         *        It automatically dispatches to the correct model setup based on compilation flags (e.g. KUB_MODEL_8).
         *
         * @param hwPath Path to the hardware config file.
         * @param procPath Path to the process config file.
         * @param adminPath Path to the admin config file.
         */
        static void generateDefaults(const std::string &hwPath, const std::string &procPath, const std::string &adminPath);

    private:
        // Model-specific generation routines
        static void generateModel8(const std::string &hwPath, const std::string &procPath, const std::string &adminPath);
        static void generateModel8Admin(const std::string &adminPath, uint32_t &filledCount);
        static void generateModel8Hardware(const std::string &hwPath, uint32_t &filledCount);
        static void generateModel8Process(const std::string &procPath, uint32_t &filledCount);

        // Helpers
        static kinematic_profile_t buildStepperKinematicProfile(
            const std::string &profileId,
            double targetVelMmS,
            double accelMmS2,
            uint8_t stepFraction);
    };

} // namespace Kub3::Config
