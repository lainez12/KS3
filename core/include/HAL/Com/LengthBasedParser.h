#pragma once

#include "HAL/Com/IProtocolParser.h"

namespace Kub3::HAL::Com
{

    class LengthBasedParser final : public IProtocolParser
    {
    public:
        void feedBytes(const QByteArray &rawData) override;
        void reset(void) override;
        [[nodiscard]] Optional<packet_t> tryExtractPacket(void) override;
        QByteArray buildPacket(const QByteArray &data) override;

    private:
        QByteArray m_buffer;
    };

}