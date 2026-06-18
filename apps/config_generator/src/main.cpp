#include <QCoreApplication>
#include <QDebug>

#include "ConfigGenerator.h"

int main(int argc, char *argv[])
{
    // Minimal Core app is enough since there is no GUI logic needed.
    QCoreApplication app(argc, argv);
    app.setApplicationName("KUB3 Config Generator Tool");

    QString hwPath    = KUB3_HARDWARE_CONFIG_PATH;
    QString procPath  = KUB3_PROCESS_CONFIG_PATH;
    QString adminPath = KUB3_ADMIN_CONFIG_PATH;

    // Support paths override via command line arguments
    if (argc >= 4)
    {
        hwPath    = argv[1];
        procPath  = argv[2];
        adminPath = argv[3];
    }

    qInfo() << "Using Config Paths:";
    qInfo() << "  Hardware:" << hwPath;
    qInfo() << "  Process :" << procPath;
    qInfo() << "  Admin   :" << adminPath;

    try
    {
        Kub3::Config::ConfigGenerator::generateDefaults(hwPath, procPath, adminPath);
    }
    catch (const std::exception &e)
    {
        qCritical() << "CRITICAL: Config generation failed with error:" << e.what();
        return 1;
    }
    catch (...)
    {
        qCritical() << "CRITICAL: Config generation failed due to an unknown error.";
        return 1;
    }

    return 0;
}
