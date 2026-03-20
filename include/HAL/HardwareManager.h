#pragma once

#include <QObject>
#include <QThread>
#include <vector>

#include "HAL/Com/PacketRouter.h"
#include "HAL/MCUDriver.h"
#include "HAL/MachineStatus/IMachineStatusRepo.h"
#include "utils.h"

namespace Kub3::HAL
{

    class HardwareManager : public QObject
    {
        Q_OBJECT
    public:
        explicit HardwareManager(Shared<MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~HardwareManager() override;

        void startAll();
        void stopAll();

    private:
        void setupArduino3Subsystem();
        void registerSensor(Com::PacketRouter *router, std::string &&route, Shared<Kub3::HAL::Sensors::ISensor> sensor);

    private:
        Shared<MS::IMachineStatusRepo> m_repo;

        std::vector<Unique<QThread>> m_threads;
        std::vector<Unique<MCUDriver>> m_drivers;
        std::vector<Unique<Com::PacketRouter>> m_routers;
        std::vector<Shared<Sensors::ISensor>> m_sensors;
    };

}
