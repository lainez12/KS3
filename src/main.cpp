// Libs
#include <QApplication>
#include <QThread>

// Public includes
// --- Utils & misc.
#include "utils.h"
#include "version.h"
// --- Machine state repository
#include "HAL/MachineStatus/MachineStatusRepo.h"
#include "HAL/MachineStatus/machine_status_builder.h"
// --- Communications
#include "HAL/Com/LengthBasedParser.h"
#include "HAL/Com/SerialCommunicator.h"
#include "HAL/MCUDriver.h"
// --- UI
#include "Views/MainWindow.h"

// Private includes
#include "Logger.h"

using namespace Kub3;

QtMessageHandler originalHandler = nullptr;

static void qtLogsHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Logger::write(qFormatLogMessage(type, context, msg));

    if (originalHandler)
        originalHandler(type, context, msg);
}

static void handlePacketReady(const Kub3::HAL::Com::packet_t &packet)
{
    qInfo().nospace() << "> Received packet of size: " << packet.length << " = [" << packet.payload.toHex(' ') << "]";
}

static void handleHardwareError(const QString &reason)
{
    qCritical().nospace() << "> MCUDriver hardware error: " << reason;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::init();                                           // Initilize logger (sets up log file location)
    originalHandler = qInstallMessageHandler(&qtLogsHandler); // Install log handler

    qInfo() << "Initializing Kub3Software...";
    qInfo() << "----";
    qInfo() << "--- Version:" << KUB3_VERSION_STR;
    qInfo() << "--- Kub3 model:" << KUB_MODEL_STR;
    qInfo() << "--- Log file path:" << Logger::file_location();
    qInfo() << "----";

    // Initialize machine state (sensors)
    HAL::MS::MachineStatusRepo statusRepo;
    HAL::MS::build_machine_status(statusRepo);

    QThread mcuDriverThread;
    auto serialCommunicator = std::make_unique<HAL::Com::SerialCommunicator>("/dev/ttyACM0", 115200);
    auto lenBasedParser     = std::make_unique<HAL::Com::LengthBasedParser>();
    HAL::MCUDriver mcuDriver(std::move(serialCommunicator), std::move(lenBasedParser), nullptr);

    mcuDriver.moveToThread(&mcuDriverThread);
    mcuDriver.connect(&mcuDriver, &HAL::MCUDriver::packetReady, &handlePacketReady);
    mcuDriver.connect(&mcuDriver, &HAL::MCUDriver::hardwareError, &handleHardwareError);
    mcuDriver.start();
    mcuDriverThread.start();

    // UI
    MainWindow w;

    w.show();

    const int ret = app.exec();

    mcuDriverThread.exit();
    if (!mcuDriverThread.wait())
        mcuDriverThread.terminate();

    return ret;
}
