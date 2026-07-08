#pragma once

#include <string>

#include "./conf.h"

namespace Kub3::Config
{

    class ConfigLoader
    {
    public:
        // Parses the file located at `filePath` as a `hardware_config_t QSettings` file.
        // Throws std::runtime_error on failure.
        static hardware_config_t loadHardwareConfig(const std::string &filePath, bool strict = true);
        // Parses the file located at `filePath` as a `process_config_t QSettings` file.
        // Throws std::runtime_error on failure.
        static process_config_t loadProcessConfig(const std::string &filePath, bool strict = true);
        // Parses the file located at `filePath` as a `admin_config_t QSettings` file.
        // Throws std::runtime_error on failure.
        static admin_config_t loadAdminConfig(const std::string &filePath, bool strict = true);
    };

} // namespace Kub3::Config