// Libs
#include <QApplication>
#include <QObject>
#include <QThread>

// Public includes
// --- Utils & misc.
#include "utils.h"
#include "version.h"
// --- Hardware manager
#include "HAL/HardwareManager.h"
// --- Machine state repository
#include "HAL/MachineStatus/MachineStatusRepo.h"
// --- UI
#include "Views/MainWindow.h"

// Private includes
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

    // Create machine state (sensors)
    auto machineStatusRepo = std::make_shared<HAL::MS::MachineStatusRepo>();
    // Create hardware manager
    HAL::HardwareManager hwManager(machineStatusRepo);
    // Create UI
    MainWindow w;

    hwManager.startAll();
    w.show();

    const int ret = app.exec();

    hwManager.stopAll(); // Gracefull shutdown

    return ret;
}
