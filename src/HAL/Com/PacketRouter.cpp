#include <QDebug>

#include <HAL/Com/PacketRouter.h>

namespace Kub3::HAL::Com
{

    PacketRouter::PacketRouter(KeyExtractor extractor, QObject *parent) :
        QObject(parent),
        m_extractor(std::move(extractor))
    {
    }

    void PacketRouter::registerRoute(const std::string &key, PacketHandler handler)
    {
        qInfo() << "[PacketRouter] route linked:" << QByteArray(key).toHex(' ');
        m_routes[key] = std::move(handler);
    }

    void PacketRouter::ps_routePacket(const packet_t &packet)
    {
        if (packet.payload.isEmpty() || !m_extractor)
            return;

        std::string_view routeKey = m_extractor(packet);

#if defined(BUILD_DEBUG)
        // qDebug() << "[PacketRouter] recv route:" << QByteArray(routeKey).toHex(' ') << " | full packet:" << packet.payload.toHex(' ');
#endif
        if (auto it = m_routes.find<std::string_view>(routeKey); it != m_routes.end())
        {
            if (it->second)
                it->second(packet.payload.sliced(routeKey.length())); // Forward the key-trimmed payload to the specific sensor
            else
                qWarning() << "[PacketRouter] Packet recognized but handler is NULL:" << packet.payload.toHex(' ');
        }
        else
        {
            qWarning() << "[PacketRouter] Unknown packet route for:" << packet.payload.toHex(' ');
        }
    }

}
