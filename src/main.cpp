// Libs
#include <QApplication>
#include <QObject>
#include <QThread>

#include "ApplicationBuilder.h"
#include "Logger.h"

using namespace Kub3;

static QtMessageHandler originalHandler = nullptr;

static void qtLogsHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Logger::write(qFormatLogMessage(type, context, msg));

    if (originalHandler)
        originalHandler(type, context, msg);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::init();                                           // Initilize logger (sets up log file location)
    originalHandler = qInstallMessageHandler(&qtLogsHandler); // Install log handler

    qInfo() << "========================================";
    qInfo() << "  KUB3 Control Software";
    qInfo() << "  Version:" << KUB3_VERSION_STR;
    qInfo() << "  Model:" << KUB_MODEL_STR;
    qInfo() << "  Log path:" << Logger::file_location();
    qInfo() << "========================================";

    ApplicationBuilder appBuilder;

    return appBuilder
        .loadConfigurations("/tmp/hardware.ini", "/tmp/process.ini")
        .buildHardwareTier()
        .buildLogicTier()
        .buildUserInterfaceTier()
        .wireArchitecture()
        .run(app);
}
