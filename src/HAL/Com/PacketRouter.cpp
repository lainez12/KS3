#include <QDebug>

#include "HAL/Com/PacketRouter.h"

namespace Kub3::HAL::Com
{

    PacketRouter::PacketRouter(KeyExtractor extractor, QObject *parent) :
        QObject(parent),
        m_extractor(std::move(extractor))
    {
    }

    void PacketRouter::registerRoute(const std::string &key, Shared<Sensors::ISensor> sensor)
    {
        qInfo() << "[PacketRouter] route linked:" << QByteArray(key).toHex(' ');
        m_routes[key] = std::move(sensor);
    }

    void PacketRouter::routePacket(const Kub3::HAL::Com::packet_t &packet)
    {
        if (packet.payload.isEmpty() || !m_extractor)
            return;

        std::string_view routeKey = m_extractor(packet);

        // qInfo() << "[PacketRouter] recv route:" << QByteArray(routeKey).toHex(' ');
        if (auto it = m_routes.find<std::string_view>(routeKey); it != m_routes.end())
        {
            it->second->processData(packet.payload.sliced(routeKey.length())); // Forward the key-trimmed payload to the specific sensor
        }
        else
        {
            qWarning() << "[PacketRouter] Unknown packet route for:" << packet.payload.toHex(' ');
        }
    }

}
