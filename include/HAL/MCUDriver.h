#pragma once

#include <QObject>
#include <memory>

#include "HAL/Com/ICommunicator.h"
#include "HAL/Com/IProtocolParser.h"
#include "utils.h"

namespace Kub3::HAL
{

    class MCUDriver : public QObject
    {
        Q_OBJECT
    public:
        // Injection of both the communicator and the parser
        explicit MCUDriver(Unique<Com::ICommunicator> comm, Unique<Com::IProtocolParser> parser, QObject *parent = nullptr);
        ~MCUDriver();

    signals:
        // This is the clean, validated packet sent to your Services/FSM!
        void packetReady(const Com::packet_t &packet);
        void hardwareError(const QString &reason);

    public slots:
        void start(void);                     // Called to open the port
        void stop(void);                      // Called to close the port
        void sendCommand(QByteArray payload); // No reference

    private slots:
        // Triggered automatically when the Communicator emits dataReceived
        void onRawDataReceived(const QByteArray &rawBytes);

    protected:
        Unique<Com::ICommunicator> m_comm;
        Unique<Com::IProtocolParser> m_parser;
    };

}