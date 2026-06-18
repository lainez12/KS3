#pragma once

#include <QByteArray>
#include <optional>

#include "utils.h"

namespace Kub3::HAL::Com
{

    typedef struct packet_s {
        bool isValid;
        uint8_t length;
        QByteArray payload;
    } packet_t;

    class IProtocolParser
    {
    public:
        virtual ~IProtocolParser(void) = default;
        // Append raw bytes to the internal buffer
        virtual void feedBytes(const QByteArray &rawData) = 0;
        // Clear the buffer (useful if communication is lost or re-synchronized)
        virtual void reset(void) = 0;
        // Parse the buffer. Returns a Packet if a full, valid frame is found.
        // If returning a packet, it MUST remove those bytes from the internal buffer.
        [[nodiscard]] virtual Optional<packet_t> tryExtractPacket(void) = 0;
        // Build a packet from the provided data using the implented protocol.
        virtual QByteArray buildPacket(const QByteArray &data) = 0;
    };

} // namespace Kub3::HAL::Com