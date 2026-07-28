#include <QDebug>
#include <QPointer>
#include <QThread>

#include "HAL/MCUDriver.h"

namespace Kub3::HAL
{

    MCUDriver::MCUDriver(Unique<Com::ICommunicator> comm, Unique<Com::IProtocolParser> parser, QObject *parent) :
        QObject(parent),
        m_comm(std::move(comm)),
        m_parser(std::move(parser))
    {
        if (m_comm)
            m_comm->setParent(this);

        // Link the raw byte reception to our processing slot
        connect(m_comm.get(), &Com::ICommunicator::s_dataReceived, this, &MCUDriver::onRawDataReceived);
        connect(m_comm.get(), &Com::ICommunicator::s_connected, this, &MCUDriver::s_connected); // Proxy connected signal
        connect(
            m_comm.get(), &Com::ICommunicator::s_connectionLost, this,
            [this]() {
                emit s_hardwareError("Connection Lost");
                emit s_connectionLost(); // Proxy connection lost signal
            });
    }

    MCUDriver::~MCUDriver()
    {
        this->ps_stop();
    }

    void MCUDriver::ps_start()
    {
        if (!m_comm->open())
        {
            emit s_hardwareError("Failed to open communicator");
        }
    }

    void MCUDriver::ps_stop()
    {
        if (m_comm && m_comm->isOpen())
            m_comm->close();

        if (m_parser)
            m_parser->reset();
    }

    void MCUDriver::onRawDataReceived(const QByteArray &rawBytes)
    {
        // Feed the fragmented bytes into the parser
        m_parser->feedBytes(rawBytes);

        // Extract as many complete packets as possible
        while (Optional<Com::packet_t> packetOpt = m_parser->tryExtractPacket())
        {
            if (packetOpt->isValid)
            {
                // Reconstruct the full clean packet for the logger
                {
                    QByteArray fullPacket;

                    fullPacket.append(static_cast<char>(packetOpt->length));
                    fullPacket.append(packetOpt->payload);
                    emit s_parsedRxData(fullPacket); // Send contiguous bytes to log
                }
                // Emit the full, validated packet to the subscribers
                emit s_packetReady(packetOpt.value());
            }
            else
                qCritical() << "Corrupted packet parsed";
        }
    }

    void MCUDriver::sendCommand(QByteArray payload, Qt::ConnectionType connType)
    {
        QPointer<MCUDriver> weakThis(this); // To be able to check `this` validity from the lambda

        QMetaObject::invokeMethod(
            this,
            [weakThis, payload]() {
                if (weakThis)
                {
                    QByteArray packet  = weakThis->m_parser->buildPacket(payload);
                    QByteArray logCopy = packet;

                    if (weakThis->m_comm->send(std::move(packet)))
                    {
                        emit weakThis->s_rawDataSent(logCopy); // Log only if data has been sent
                    }
                }
            },
            connType);
    }
}