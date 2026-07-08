#pragma once

#include <Config/utils.h>

namespace Kub3::Config
{

    ///////////////////////////
    // ADMIN CONFIGURATION
    ///////////////////////////

    // Top level struct for admin config
    typedef struct admin_config_s {
        bool kloe_mode = false;

        void mergeMissingFieldsFrom(const admin_config_s &source, const std::string &path, std::vector<std::string> &logs)
        {
            MERGE_FIELD(*this, source, false, kloe_mode, path, logs);
        }
    } admin_config_t;

} // Kub3::Config
