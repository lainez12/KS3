// Libs
#include <QApplication>
#include <QThread>

// Public includes
// --- Utils & misc.
#include "utils.h"
#include "version.h"
// --- Communications
#include "HAL/Com/LengthBasedParser.h"
#include "HAL/Com/SerialCommunicator.h"
#include "HAL/MCUDriver.h"
// --- UI
#include "Views/MainWindow.h"

// Private includes
#include "Logger.h"

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

    QThread mcuDriverThread;
    auto serialCommunicator = std::make_unique<Kub3::HAL::Com::SerialCommunicator>("/dev/ttyACM0", 115200);
    auto lenBasedParser     = std::make_unique<Kub3::HAL::Com::LengthBasedParser>();
    Kub3::HAL::MCUDriver mcuDriver(std::move(serialCommunicator), std::move(lenBasedParser), nullptr);

    mcuDriver.moveToThread(&mcuDriverThread);
    mcuDriver.connect(&mcuDriver, &Kub3::HAL::MCUDriver::packetReady, &handlePacketReady);
    mcuDriver.connect(&mcuDriver, &Kub3::HAL::MCUDriver::hardwareError, &handleHardwareError);
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
