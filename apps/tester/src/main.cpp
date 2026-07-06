#include <QApplication>

#include "Logger.h"
#include "version.h"
#include <ApplicationBuilder.h>

static QtMessageHandler originalHandler = nullptr;

static void qtLogsHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Logger::write(qFormatLogMessage(type, context, msg));

    if (originalHandler)
        originalHandler(type, context, msg);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    app.setApplicationName("KUB3 Motor Tester");
    app.setApplicationVersion(KUB3_VERSION_STR);

    Logger::init();                                           // Initilize logger (sets up log file location)
    originalHandler = qInstallMessageHandler(&qtLogsHandler); // Install log handler

    qInfo() << "========================================";
    qInfo() << "  KUB3 MOTOR TESTER TOOL";
    qInfo() << "  Version:" << KUB3_VERSION_STR;
    qInfo() << "  Hardware Config Path:" << KUB3_HARDWARE_CONFIG_PATH;
    qInfo() << "  Process Config Path:" << KUB3_PROCESS_CONFIG_PATH;
    qInfo() << "  Admin Config Path:" << KUB3_ADMIN_CONFIG_PATH; // Unused for now
    qInfo() << "  Log path:" << Logger::file_location();
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
