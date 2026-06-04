#pragma once

#include <string>

#include "./conf.h"

namespace Kub3::Config
{

    class ConfigLoader
    {
    public:
        // Parses the hardware.ini file. Throws std::runtime_error on failure.
        static hardware_config_t loadHardwareConfig(const std::string &filePath);
        // Parses the process.ini file. Throws std::runtime_error on failure.
        static process_config_t loadProcessConfig(const std::string &filePath);
    };

} // namespace Kub3::Config