#pragma once

#include <QDebug>
#include <QObject>
#include <iostream>

#include "HAL/Sensors/ISensor.h"
#include "IProtocolParser.h"

namespace Kub3::HAL::Com
{

    class PacketRouter : public QObject
    {
        Q_OBJECT
    public:
        using KeyExtractor  = std::function<std::string_view(const packet_t &)>;
        using PacketHandler = std::function<void(const QByteArray &)>;

        explicit PacketRouter(KeyExtractor extractor, QObject *parent = nullptr);

        void registerRoute(const std::string &key, PacketHandler handler);

    public slots:
        void ps_routePacket(const packet_t &packet);

    private:
        using RouteMap = std::unordered_map<std::string, PacketHandler, Utils::StringViewHash, std::equal_to<>>;

        RouteMap m_routes;
        KeyExtractor m_extractor;
    };

}
