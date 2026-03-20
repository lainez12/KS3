#pragma once

#include <QDebug>
#include <QObject>
#include <iostream>

#include "HAL/Sensors/ISensor.h"
#include "IProtocolParser.h"

namespace Kub3::HAL::Com
{

    struct StringViewHash
    {
        using is_transparent = void;

        [[nodiscard]] size_t operator()(std::string_view sv) const
        {
            return std::hash<std::string_view>{}(sv);
        }
    };

    class PacketRouter : public QObject
    {
        Q_OBJECT
    public:
        using KeyExtractor = std::function<std::string_view(const packet_t &)>;

        explicit PacketRouter(KeyExtractor extractor, QObject *parent = nullptr);

        void registerRoute(const std::string &key, Shared<Sensors::ISensor> sensor);

    public slots:
        void routePacket(const Kub3::HAL::Com::packet_t &packet);

    private:
        using RouteMap = std::unordered_map<std::string, Shared<Sensors::ISensor>, StringViewHash, std::equal_to<>>;

        RouteMap m_routes;
        KeyExtractor m_extractor;
    };

}
