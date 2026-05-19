#pragma once

#include <string>

#include "machine_config.h"

namespace Kub3::Config
{

    class ConfigSaver
    {
    public:
        // Serializes and atomically saves the hardware configuration.
        // Throws std::runtime_error on disk or permission failures.
        static void saveHardwareConfig(const hardware_config_t &config, const std::string &targetPath);

        // Serializes and atomically saves the process configuration.
        // Throws std::runtime_error on disk or permission failures.
        static void saveProcessConfig(const process_config_t &config, const std::string &targetPath);
    };

} // namespace Kub3::Config
