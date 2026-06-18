#pragma once

#include <QString>

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
        static void generateDefaults(const QString &hwPath, const QString &procPath, const QString &adminPath);

    private:
        // Model-specific generation routines
        static void generateModel8(const QString &hwPath, const QString &procPath, const QString &adminPath);
    };

} // namespace Kub3::Config
