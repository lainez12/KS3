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
#include <HAL/Vision/ICamera.h>
#include <HAL/Vision/identifiers.h>
#include <utils.h>

namespace Kub3::HAL
{

    struct MCUSubsystemNode {
        Unique<QThread> thread;
        Shared<MCUDriver> driver;
        Unique<Com::PacketRouter> router;
    };

    struct CameraSubsystemNode {
        Unique<QThread> thread;
        Shared<Vision::ICamera> camera;
    };

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
        [[nodiscard]] const std::vector<std::string> &getRegisteredMotorIds() const
        {
            return m_registeredMotorIds;
        }
        [[nodiscard]] const std::vector<std::string> &getRegisteredFocalIds() const
        {
            return m_registeredFocalIds;
        }

    signals:
        void s_hardwarePowerOffSent(void);
        void s_cameraFrameReady(const QString &cameraId, const QImage &frame);

    public slots:
        void ps_reconnectMCUSubsystem(const QString &subsystemId);
        void ps_reconnectCameraSubsystem(const QString &cameraId);
        void ps_updateCameraParameter(const QString &cameraId, Vision::CameraParamKind kind, Vision::CameraParam value);
        void ps_powerOff(void);

    private:
#if defined(KUB_MODEL_8)
        // Arduino1 subsystem
        void setupArduino1Subsystem(const Config::hardware_config_t &config);
        void createArduino1Sensors(Com::PacketRouter *router);
        void createArduino1Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver);

        // Arduino2 subsystem
        void setupArduino2Subsystem(const Config::hardware_config_t &config);
        void createArduino2Sensors(Com::PacketRouter *router);
        void createArduino2Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver);

        // Arduino3 subsystem
        void setupArduino3Subsystem(const Config::hardware_config_t &config);
        void createArduino3Sensors(Com::PacketRouter *router);
        void createArduino3Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver);

        // Cameras subsystem
        void setupCamerasSubsystem(const Config::hardware_config_t &config);
#endif // defined(KUB_MODEL_8)

        void registerSensor(Com::PacketRouter *router, std::string &&route, Shared<Kub3::HAL::Sensors::ISensor> sensor);
        Shared<Act::StepperMotor> createStepperMotor(
            const Config::hardware_config_t &config,
            const QString &motorId,
            uint8_t byteId,
            Algorithms::Kinematic::KinematicGeneratorKind kineGenKind,
            const std::shared_ptr<MCUDriver> &driver,
            const std::string &encoderId);

    private:
        Shared<MS::IMachineStatusRepo> m_repo;
        Shared<Act::ActuatorRegistry> m_actuatorRegistry;

        std::unordered_map<QString, MCUSubsystemNode> m_subsystems;
        std::unordered_map<QString, CameraSubsystemNode> m_cameras;
        std::vector<Shared<Sensors::ISensor>> m_sensors; // TODO: What for ?

        // Registered actuators ids
        std::vector<std::string> m_registeredMotorIds;
        std::vector<std::string> m_registeredFocalIds;
    };

}
