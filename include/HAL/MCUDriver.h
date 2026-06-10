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

        void sendCommand(QByteArray payload, Qt::ConnectionType connType = Qt::AutoConnection);

    signals:
        void s_packetReady(const Com::packet_t &packet);
        void s_hardwareError(const QString &reason);

        // Communicator proxy signals
        void s_connected(void);
        void s_connectionLost(void);

    public slots:
        void ps_start(void); // Called to open the port
        void ps_stop(void);  // Called to close the port

    private slots:
        // Triggered automatically when the Communicator emits s_dataReceived
        void onRawDataReceived(const QByteArray &rawBytes);

    protected:
        Unique<Com::ICommunicator> m_comm;
        Unique<Com::IProtocolParser> m_parser;
    };

}