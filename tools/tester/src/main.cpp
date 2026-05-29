#include <ApplicationBuilder.h>
#include <QApplication>

#include "version.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setApplicationName("KUB3 Motor Tester");
    app.setApplicationVersion(KUB3_VERSION_STR);

    qInfo() << "========================================";
    qInfo() << "  KUB3 MOTOR TESTER TOOL";
    qInfo() << "  Version:" << KUB3_VERSION_STR;
    qInfo() << "  Hardware Config Path:" << KUB3_HARDWARE_CONFIG_PATH;
    qInfo() << "  Process Config Path:" << KUB3_PROCESS_CONFIG_PATH;
    qInfo() << "========================================";

    Kub3::Tools::Tester::ApplicationBuilder builder;

    // Path resolution matching your CMake definitions
    builder.loadConfigurations(KUB3_HARDWARE_CONFIG_PATH, KUB3_PROCESS_CONFIG_PATH);
    builder.buildHardwareTier();
    builder.buildLogicTier();
    builder.buildUserInterfaceTier();
    builder.wireArchitecture();

    return builder.run(app);
}
