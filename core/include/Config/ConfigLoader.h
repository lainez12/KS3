#pragma once

#include <string>
#include <vector>

#include "./conf.h"

namespace Kub3::Config
{

    using Logs = std::vector<std::string>;

    class ConfigLoader
    {
    public:
        static hardware_config_t loadHardwareConfig(const std::string &filePath, bool strict = true, Logs *logs = nullptr);
        static process_config_t loadProcessConfig(const std::string &filePath, bool strict = true, Logs *logs = nullptr);
        static admin_config_t loadAdminConfig(const std::string &filePath, bool strict = true, Logs *logs = nullptr);
    };

} // namespace Kub3::Config
