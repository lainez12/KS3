#include <QDebug>

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
        connect(m_comm.get(), &Com::ICommunicator::dataReceived, this, &MCUDriver::onRawDataReceived);
        connect(
            m_comm.get(),
            &Com::ICommunicator::connectionLost,
            this,
            [this]()
            { emit s_hardwareError("Connection Lost"); });
    }

    MCUDriver::~MCUDriver()
    {
        this->ps_stop();
        // TODO: notify disconnection to MCU ?
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
                emit s_packetReady(packetOpt.value()); // Emit the full, validated packet to the subscribers
            else
                qCritical() << "Corrupted packet parsed"; // TODO: better logging & handling
        }
    }

    void MCUDriver::ps_sendCommand(QByteArray payload)
    {
        m_comm->send(std::move(payload));
    }
}