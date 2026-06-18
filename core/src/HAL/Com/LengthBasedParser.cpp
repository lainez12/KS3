#include <QDebug>

#include "HAL/Com/LengthBasedParser.h"

namespace Kub3::HAL::Com
{

    void LengthBasedParser::feedBytes(const QByteArray &rawData)
    {
        m_buffer.append(rawData);
    }

    void LengthBasedParser::reset()
    {
        m_buffer.clear();
    }

    Optional<packet_t> LengthBasedParser::tryExtractPacket()
    {
        if (m_buffer.isEmpty())
            return std::nullopt;

        const uint8_t packetLen = m_buffer[0];

        if (m_buffer.size() - 1 < packetLen)
            return std::nullopt; // Wait for the rest of the packet to arrive

        QByteArray payload = m_buffer.sliced(1, packetLen);

        m_buffer.remove(0, packetLen + 1); // +1 for length byte

        return packet_t{
            .isValid = true,
            .length  = packetLen,
            .payload = payload,
        };
    }

    QByteArray LengthBasedParser::buildPacket(const QByteArray &data)
    {
        QByteArray packet;

        packet.reserve(sizeof(uint8_t) + data.length());
        packet.append(static_cast<uint8_t>(data.length()));
        packet.append(data);
        return packet;
    }

} // namespace Kub3::HAL::Com
