#pragma once

#include "HAL/Com/IProtocolParser.h"

namespace Kub3::HAL::Com
{

    class LengthBasedParser : public IProtocolParser
    {
    public:
        void feedBytes(const QByteArray &rawData) override final;
        [[nodiscard]] Optional<packet_t> tryExtractPacket(void) override final;
        void reset(void) override final;

    private:
        QByteArray m_buffer;
    };

}