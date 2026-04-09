#pragma once

#include <QObject>
#include <QThread>
#include <vector>

#include <Algorithms/Kinematic/utils.h>
#include <Config/ConfigLoader.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Motors/StepperMotor.h>
#include <HAL/Com/PacketRouter.h>
#include <HAL/MCUDriver.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <utils.h>

namespace Kub3::HAL
{

    class HardwareManager : public QObject
    {
        Q_OBJECT
    public:
        explicit HardwareManager(Shared<MS::IMachineStatusRepo> repo, const Config::hardware_config_t &hardwareConfig, QObject *parent = nullptr);
        ~HardwareManager() override;

        void startAll();
        void stopAll();

        [[nodiscard]] inline Shared<Act::ActuatorRegistry> getActuatorRegistry() const
        {
            return m_actuatorRegistry;
        }

    private:
#if defined(KUB_MODEL_8)
        void setupArduino2Subsystem(const Config::hardware_config_t &config);

        void setupArduino3Subsystem(const Config::hardware_config_t &config);
        void createArduino3Sensors(Com::PacketRouter *router);
        void createArduino3Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver);
#endif // defined(KUB_MODEL_8)

        void registerSensor(Com::PacketRouter *router, std::string &&route, Shared<Kub3::HAL::Sensors::ISensor> sensor);
        Shared<Act::StepperMotor> createStepperMotor(
            const Config::hardware_config_t &config,
            const std::string &motorId,
            uint8_t byteId,
            Algorithms::Kinematic::KinematicGeneratorKind kineGenKind,
            const std::shared_ptr<MCUDriver> &driver,
            const std::string &encoderId);

    private:
        Shared<MS::IMachineStatusRepo> m_repo;
        Shared<Act::ActuatorRegistry> m_actuatorRegistry;

        std::vector<Unique<QThread>> m_threads;
        std::vector<Shared<MCUDriver>> m_drivers;
        std::vector<Unique<Com::PacketRouter>> m_routers;
        std::vector<Shared<Sensors::ISensor>> m_sensors;
    };

}
