#include <QDebug>

#include <Algorithms/Kinematic/IKinematicGenerator.h>
#include <HAL/Actuators/Lights/UVExposureHead.h>
#include <HAL/Actuators/Valves/SolenoidValve.h>
#include <HAL/Com/LengthBasedParser.h>
#include <HAL/Com/SerialCommunicator.h>
#include <HAL/HardwareManager.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/Sensors/Sensor.h>

using namespace std::string_literals;

// Packet key extractors
static std::string_view arduino2KeyExtractor(const Kub3::HAL::Com::packet_t &packet);
static std::string_view arduino3KeyExtractor(const Kub3::HAL::Com::packet_t &packet);
// Payload parsers
static bool limitSwitchParser(const QByteArray &d);
static bool valveStatusParser(const QByteArray &d);
static bool pressureSensorParser(const QByteArray &d);
static int32_t temperatureParser(const QByteArray &d);
static uint16_t forceSensorParser(const QByteArray &d);
static bool forceSensorEnabledParser(const QByteArray &d);
static int32_t encoderValueParser(const QByteArray &d);
static bool physicalButtonParser(const QByteArray &d);

namespace Kub3::HAL
{

    HardwareManager::HardwareManager(Shared<MS::IMachineStatusRepo> repo, const Config::hardware_config_t &config, QObject *parent) :
        QObject(parent),
        m_repo(std::move(repo)),
        m_actuatorRegistry(std::make_shared<Act::ActuatorRegistry>())
    {
// TODO: Build the machine based on the CMake configuration
#if defined(KUB_MODEL_8)
        // setupArduino1Subsystem(config);
        setupArduino2Subsystem(config);
        setupArduino3Subsystem(config);
#endif
    }

    HardwareManager::~HardwareManager()
    {
        stopAll();
    }

    void HardwareManager::startAll()
    {
        for (auto &[key, subsys] : m_subsystems)
        {
            subsys.thread->start();
            QMetaObject::invokeMethod(subsys.driver.get(), &MCUDriver::ps_start);
        }
    }

    void HardwareManager::stopAll()
    {
        for (auto &[key, subsys] : m_subsystems)
        {
            if (!subsys.thread || !subsys.thread->isRunning())
                continue;

            QMetaObject::invokeMethod(subsys.driver.get(), &MCUDriver::ps_stop, Qt::QueuedConnection);
            subsys.thread->exit(0);
            if (!subsys.thread->wait(2000))
            {
                subsys.thread->terminate(); // Force kill
                subsys.thread->wait();
            }
        }
    }

    void HardwareManager::ps_reconnectSubsystem(const QString &subsystemId)
    {
        const std::string id = subsystemId.toStdString();

        if (auto it = m_subsystems.find(id); it != m_subsystems.end())
        {
            auto &subsys = it->second;

            qInfo() << "HardwareManager: Reconnecting targeted subsystem:" << id;
            if (!subsys.thread)
            {
                qCritical() << "HardwareManager: Attempt to reconnect using invalid thread pointer:" << id;
                return;
            }

            if (subsys.thread->isRunning()) // If thread running
            {
                // Gracefully stop the driver
                QMetaObject::invokeMethod(subsys.driver.get(), &MCUDriver::ps_stop, Qt::QueuedConnection);
                // Restart the driver
                QMetaObject::invokeMethod(subsys.driver.get(), &MCUDriver::ps_start, Qt::QueuedConnection);
            }
        }
        else
        {
            qWarning() << "HardwareManager: Requested restart for unknown subsystem:" << id;
        }
    }

    void HardwareManager::ps_powerOff(void)
    {
        if (auto it = m_subsystems.find(MCU_ARDUINO2_ID); it != m_subsystems.end())
        {
            MCUSubsystemNode &node = it->second;
            QByteArray payload(1, 'E');

            QMetaObject::invokeMethod(node.driver.get(), &MCUDriver::ps_sendCommand, Qt::BlockingQueuedConnection, payload);
            emit s_hardwarePowerOffSent(); // Triggers device power off
        }
        else
        {
            qCritical() << "HardwareManager: Fatal Error. Power Control MCU not found.";
        }
    }

#if defined(KUB_MODEL_8)

    // ======================================================
    // KUB3-8i HAL instanciation functions (split by MCU)
    // ======================================================

    // --- Arduino 2 HAL instanciation helpers

    void HardwareManager::setupArduino2Subsystem(const Config::hardware_config_t &config)
    {
        using namespace Kub3::HAL::Sensors;

        // Create thread & Driver for Arduino3
        auto thread         = std::make_unique<QThread>();                                       // Driver thread
        auto comms          = std::make_unique<Com::SerialCommunicator>("/dev/ttyACM1", 115200); // TODO: get port from settings file
        auto parser         = std::make_unique<Com::LengthBasedParser>();
        auto arduino2Driver = std::make_shared<MCUDriver>(std::move(comms), std::move(parser));
        auto router         = std::make_unique<Com::PacketRouter>(&arduino2KeyExtractor);

        // Initialize MCU driver connection "sensor". Doesn't need a parser.
        m_sensors.push_back(std::make_shared<HAL::Sensors::Sensor<bool>>(m_repo, MCU_ARDUINO2_READY, false, nullptr));

        // Instanciate sensors and actuators software representations
        this->createArduino2Sensors(router.get());
        this->createArduino2Actuators(config, arduino2Driver);

        // Move MCUDriver to its own thread
        arduino2Driver->moveToThread(thread.get());

        // Wire MCUDriver connection status signals -> Machine Status Repo value update
        QObject::connect(arduino2Driver.get(), &MCUDriver::s_connected, [&]() { m_repo->setSensorRaw(MCU_ARDUINO2_READY, true); });
        QObject::connect(arduino2Driver.get(), &MCUDriver::s_connectionLost, [&]() { m_repo->setSensorRaw(MCU_ARDUINO2_READY, false); });
        // Wire MCUDriver -> Router
        QObject::connect(arduino2Driver.get(), &MCUDriver::s_packetReady, router.get(), &Com::PacketRouter::ps_routePacket);

        // Store in lifecycle manager
        m_subsystems[MCU_ARDUINO2_ID] = MCUSubsystemNode{
            .thread = std::move(thread),
            .driver = std::move(arduino2Driver),
            .router = std::move(router),
        };
    }

    void HardwareManager::createArduino2Sensors(Com::PacketRouter *router)
    {
        using namespace Kub3::HAL::Sensors;

        // ===========================================
        // UPWARD PIPELINE (Hardware --> Software)
        // ===========================================

        // Create Sensors
        // --- Physical buttons
        auto emergencyStopBtn = std::make_shared<Sensor<bool>>(m_repo, EMERGENCY_STOP_BUTTON, false, &physicalButtonParser);
        auto powerOffBtn      = std::make_shared<Sensor<bool>>(m_repo, POWER_OFF_BUTTON, false, &physicalButtonParser);
        // --- Limit switches
        auto camerasDeckFrontLimit = std::make_shared<Sensor<bool>>(m_repo, DECK_FRONT_LIMIT, false, &limitSwitchParser);
        auto camerasDeckBackLimit  = std::make_shared<Sensor<bool>>(m_repo, DECK_BACK_LIMIT, false, &limitSwitchParser);
        auto ardkoFrontLeftLimit   = std::make_shared<Sensor<bool>>(m_repo, DECK_BACK_LIMIT, false, &limitSwitchParser);
        auto ardkoFrontRightLimit  = std::make_shared<Sensor<bool>>(m_repo, DECK_BACK_LIMIT, false, &limitSwitchParser);
        auto ardkoBackLeftLimit    = std::make_shared<Sensor<bool>>(m_repo, DECK_BACK_LIMIT, false, &limitSwitchParser);
        auto ardkoBackRightLimit   = std::make_shared<Sensor<bool>>(m_repo, DECK_BACK_LIMIT, false, &limitSwitchParser);
        // --- Solenoid valves statii
        auto maskVacuumValveStatus         = std::make_shared<Sensor<bool>>(m_repo, MASK_VACUUM_VALVE_STATUS, false, &valveStatusParser);
        auto waferVacuumValveStatus        = std::make_shared<Sensor<bool>>(m_repo, WAFER_VACUUM_VALVE_STATUS, false, &valveStatusParser);
        auto waferCompressedAirValveStatus = std::make_shared<Sensor<bool>>(m_repo, WAFER_COMPRESSED_AIR_VALVE_STATUS, false, &valveStatusParser);
        // --- Pressure sensors
        auto maskVacuumActive         = std::make_shared<Sensor<bool>>(m_repo, MASK_VACUUM_ACTIVE, false, &pressureSensorParser);
        auto waferVacuumActive        = std::make_shared<Sensor<bool>>(m_repo, WAFER_VACUUM_ACTIVE, false, &pressureSensorParser);
        auto waferCompressedAirActive = std::make_shared<Sensor<bool>>(m_repo, WAFER_COMPRESSED_AIR_ACTIVE, false, &pressureSensorParser);
        // --- Temperatures
        auto internalTemperature = std::make_shared<Sensor<int32_t>>(m_repo, INTERNAL_TEMPERATURE, INT32_MIN, &temperatureParser);
        auto externalTemperature = std::make_shared<Sensor<int32_t>>(m_repo, EXTERNAL_TEMPERATURE, INT32_MIN, &temperatureParser);
        // --- Encoders
        auto camerasDeckEncoder = std::make_shared<Sensor<int32_t>>(m_repo, DECK_MOTOR_ENCODER, INT32_MIN, &encoderValueParser);

        // TODO: add "Fans voltage" & "LEDs voltages"

        // Register sensors
        // --- Physical buttons
        this->registerSensor(router, "E\x7F\x7F"s, std::move(emergencyStopBtn));
        this->registerSensor(router, "EE"s, std::move(powerOffBtn));
        // --- Limit switches
        this->registerSensor(router, "C1F"s, std::move(camerasDeckFrontLimit));
        this->registerSensor(router, "C1B"s, std::move(camerasDeckBackLimit));
        this->registerSensor(router, "K1", std::move(ardkoFrontLeftLimit));
        this->registerSensor(router, "K2", std::move(ardkoFrontRightLimit));
        this->registerSensor(router, "K3", std::move(ardkoBackLeftLimit));
        this->registerSensor(router, "K4", std::move(ardkoBackRightLimit));
        // --- Solenoid valves statii
        this->registerSensor(router, "VEM"s, std::move(maskVacuumValveStatus));
        this->registerSensor(router, "VEW"s, std::move(waferVacuumValveStatus));
        this->registerSensor(router, "VVAC"s, std::move(waferCompressedAirValveStatus));
        // --- Pressure sensors
        this->registerSensor(router, "VCM"s, std::move(maskVacuumActive));
        this->registerSensor(router, "VCW"s, std::move(waferVacuumActive));
        this->registerSensor(router, "VAC"s, std::move(waferCompressedAirActive));
        // --- Temperatures
        this->registerSensor(router, "IT0"s, std::move(internalTemperature));
        this->registerSensor(router, "IT1"s, std::move(externalTemperature));
        // --- Encoders
        // TODO: register deck's encoder when communication pattern is defined
    }

    void HardwareManager::createArduino2Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver)
    {
        using namespace Algorithms::Kinematic;

        // ===========================================
        // DOWNWARD PIPELINE (Software --> Hardware)
        // ===========================================

        /// --- Motors
        auto camerasDeckMotor = createStepperMotor(config, DECK_MOTOR, 'F', KinematicGeneratorKind::TRAPEZOIDAL, driver, DECK_MOTOR_ENCODER);
        /// --- Valves
        auto maskVacuumValve         = std::make_shared<Act::SolenoidValve>(MASK_VACUUM_VALVE, "VEM14095", "VEM00", driver);
        auto waferVacuumValve        = std::make_shared<Act::SolenoidValve>(WAFER_VACUUM_VALVE, "VEW14095", "VEW00", driver);
        auto waferCompressedAirValve = std::make_shared<Act::SolenoidValve>(WAFER_COMPRESSED_AIR_VALVE, "AC1", "AC0", driver);
        /// --- Exposure head
        auto exposureHead = std::make_shared<Act::UVExposureHead>(UV_EXPOSURE_HEAD, driver);

        /// --- Motors
        m_actuatorRegistry->registerActuator(std::move(camerasDeckMotor));
        /// --- Valves
        m_actuatorRegistry->registerActuator(std::move(maskVacuumValve));
        m_actuatorRegistry->registerActuator(std::move(waferVacuumValve));
        m_actuatorRegistry->registerActuator(std::move(waferCompressedAirValve));
        /// --- Exposure head
        m_actuatorRegistry->registerActuator(std::move(exposureHead));
    }

    // --- Arduino 3 HAL instanciation helpers

    void HardwareManager::setupArduino3Subsystem(const Config::hardware_config_t &config)
    {
        // Create thread & Driver for Arduino3
        auto thread         = std::make_unique<QThread>();                                       // Driver thread
        auto comms          = std::make_unique<Com::SerialCommunicator>("/dev/ttyACM0", 115200); // TODO: get port from settings file
        auto parser         = std::make_unique<Com::LengthBasedParser>();
        auto arduino3Driver = std::make_shared<MCUDriver>(std::move(comms), std::move(parser));
        auto router         = std::make_unique<Com::PacketRouter>(&arduino3KeyExtractor);

        // Initialize MCU driver connection "sensor". Doesn't need a parser.
        m_sensors.push_back(std::make_shared<HAL::Sensors::Sensor<bool>>(m_repo, MCU_ARDUINO3_READY, false, nullptr));

        // Instanciate sensors and actuators software representations
        this->createArduino3Sensors(router.get());
        this->createArduino3Actuators(config, arduino3Driver);

        // Move MCUDriver to its own thread
        arduino3Driver->moveToThread(thread.get());

        // Wire MCUDriver connection status signals -> Machine Status Repo value update
        QObject::connect(arduino3Driver.get(), &MCUDriver::s_connected, [&]() { m_repo->setSensorRaw(MCU_ARDUINO3_READY, true); });
        QObject::connect(arduino3Driver.get(), &MCUDriver::s_connectionLost, [&]() { m_repo->setSensorRaw(MCU_ARDUINO3_READY, false); });
        // Wire MCUDriver -> Router
        QObject::connect(arduino3Driver.get(), &MCUDriver::s_packetReady, router.get(), &Com::PacketRouter::ps_routePacket);

        // Store in lifecycle manager
        m_subsystems[MCU_ARDUINO3_ID] = MCUSubsystemNode{
            .thread = std::move(thread),
            .driver = std::move(arduino3Driver),
            .router = std::move(router),
        };
    }

    void HardwareManager::createArduino3Sensors(Com::PacketRouter *router)
    {
        using namespace Kub3::HAL::Sensors;

        // ===========================================
        // UPWARD PIPELINE (Hardware --> Software)
        // ===========================================

        // Create Sensors
        // --- Encoders
        auto zLeftEncoder  = std::make_shared<Sensor<int32_t>>(m_repo, Z_LEFT_ENCODER, static_cast<int32_t>(0), &encoderValueParser);
        auto zRightEncoder = std::make_shared<Sensor<int32_t>>(m_repo, Z_RIGHT_ENCODER, static_cast<int32_t>(0), &encoderValueParser);
        auto zBackEncoder  = std::make_shared<Sensor<int32_t>>(m_repo, Z_BACK_ENCODER, static_cast<int32_t>(0), &encoderValueParser);
        auto maskEncoder   = std::make_shared<Sensor<int32_t>>(m_repo, MASK_ENCODER, static_cast<int32_t>(0), &encoderValueParser);
        auto waferEncoder  = std::make_shared<Sensor<int32_t>>(m_repo, WAFER_ENCODER, static_cast<int32_t>(0), &encoderValueParser);
        // --- Limit switches
        auto zLeftHighLimit  = std::make_shared<Sensor<bool>>(m_repo, Z_LEFT_HIGH_LIMIT, false, &limitSwitchParser);
        auto zLeftLowLimit   = std::make_shared<Sensor<bool>>(m_repo, Z_LEFT_LOW_LIMIT, false, &limitSwitchParser);
        auto zRightHighLimit = std::make_shared<Sensor<bool>>(m_repo, Z_RIGHT_HIGH_LIMIT, false, &limitSwitchParser);
        auto zRightLowLimit  = std::make_shared<Sensor<bool>>(m_repo, Z_RIGHT_LOW_LIMIT, false, &limitSwitchParser);
        auto zBackHighLimit  = std::make_shared<Sensor<bool>>(m_repo, Z_BACK_HIGH_LIMIT, false, &limitSwitchParser);
        auto zBackLowLimit   = std::make_shared<Sensor<bool>>(m_repo, Z_BACK_LOW_LIMIT, false, &limitSwitchParser);
        auto cm0Limit        = std::make_shared<Sensor<bool>>(m_repo, CM0, false, &limitSwitchParser);
        auto cm1Limit        = std::make_shared<Sensor<bool>>(m_repo, CM1, false, &limitSwitchParser);
        auto cm2Limit        = std::make_shared<Sensor<bool>>(m_repo, CM2, false, &limitSwitchParser);
        auto cm3Limit        = std::make_shared<Sensor<bool>>(m_repo, CM3, false, &limitSwitchParser);
        auto cw0Limit        = std::make_shared<Sensor<bool>>(m_repo, CW0, false, &limitSwitchParser);
        auto cw1Limit        = std::make_shared<Sensor<bool>>(m_repo, CW1, false, &limitSwitchParser);
        auto cw2Limit        = std::make_shared<Sensor<bool>>(m_repo, CW2, false, &limitSwitchParser);
        auto z1Limit         = std::make_shared<Sensor<bool>>(m_repo, Z1, false, &limitSwitchParser);
        auto zWaferOnLimit   = std::make_shared<Sensor<bool>>(m_repo, WAFER_ON, false, &limitSwitchParser);
        auto z2Limit         = std::make_shared<Sensor<bool>>(m_repo, Z2, false, &limitSwitchParser);
        // --- Force sensors
        auto leftForceEn  = std::make_shared<Sensor<bool>>(m_repo, FORCE_LEFT_EN, false, &forceSensorEnabledParser);
        auto rightForceEn = std::make_shared<Sensor<bool>>(m_repo, FORCE_RIGHT_EN, false, &forceSensorEnabledParser);
        auto backForceEn  = std::make_shared<Sensor<bool>>(m_repo, FORCE_BACK_EN, false, &forceSensorEnabledParser);
        auto leftForce    = std::make_shared<Sensor<uint16_t>>(m_repo, FORCE_LEFT, static_cast<uint16_t>(0), &forceSensorParser);
        auto rightForce   = std::make_shared<Sensor<uint16_t>>(m_repo, FORCE_RIGHT, static_cast<uint16_t>(0), &forceSensorParser);
        auto backForce    = std::make_shared<Sensor<uint16_t>>(m_repo, FORCE_BACK, static_cast<uint16_t>(0), &forceSensorParser);

        // Register sensors
        // --- Encoders
        this->registerSensor(router, "1"s, std::move(zLeftEncoder));
        this->registerSensor(router, "2"s, std::move(zRightEncoder));
        this->registerSensor(router, "3"s, std::move(zBackEncoder));
        this->registerSensor(router, "4"s, std::move(maskEncoder));
        this->registerSensor(router, "5"s, std::move(waferEncoder));
        // --- Limit switches
        this->registerSensor(router, "S\x00"s, std::move(zLeftHighLimit));
        this->registerSensor(router, "S\x01"s, std::move(zLeftLowLimit));
        this->registerSensor(router, "S\x02"s, std::move(zRightHighLimit));
        this->registerSensor(router, "S\x03"s, std::move(zRightLowLimit));
        this->registerSensor(router, "S\x04"s, std::move(zBackHighLimit));
        this->registerSensor(router, "S\x05"s, std::move(zBackLowLimit));
        this->registerSensor(router, "S\x06"s, std::move(cm0Limit));
        this->registerSensor(router, "S\x07"s, std::move(cm1Limit));
        this->registerSensor(router, "S\x08"s, std::move(cm2Limit));
        this->registerSensor(router, "S\x09"s, std::move(cm3Limit));
        this->registerSensor(router, "S\x0A"s, std::move(cw0Limit));
        this->registerSensor(router, "S\x0B"s, std::move(cw1Limit));
        this->registerSensor(router, "S\x0C"s, std::move(cw2Limit));
        this->registerSensor(router, "Z1"s, std::move(z1Limit));
        this->registerSensor(router, "Z2"s, std::move(z2Limit));
        this->registerSensor(router, "Z3"s, std::move(zWaferOnLimit));
        // --- Force sensors
        this->registerSensor(router, "F?1"s, std::move(leftForceEn));
        this->registerSensor(router, "F?2"s, std::move(rightForceEn));
        this->registerSensor(router, "F?3"s, std::move(backForceEn));
        this->registerSensor(router, "F1"s, std::move(leftForce));
        this->registerSensor(router, "F2"s, std::move(rightForce));
        this->registerSensor(router, "F3"s, std::move(backForce));
    }

    void HardwareManager::createArduino3Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver)
    {
        using namespace Algorithms::Kinematic;

        // ===========================================
        // DOWNWARD PIPELINE (Software --> Hardware)
        // ===========================================

        /// --- Motors
        auto maskMotor  = createStepperMotor(config, MASK_DRAWER_MOTOR, '4', KinematicGeneratorKind::TRAPEZOIDAL, driver, MASK_ENCODER);
        auto waferMotor = createStepperMotor(config, WAFER_DRAWER_MOTOR, '5', KinematicGeneratorKind::TRAPEZOIDAL, driver, WAFER_ENCODER);

        m_actuatorRegistry->registerActuator(std::move(maskMotor));
        m_actuatorRegistry->registerActuator(std::move(waferMotor));
    }

#endif // defined(KUB_MODEL_8)

    // --- Sensors HAL instanciation helpers

    void HardwareManager::registerSensor(Com::PacketRouter *router, std::string &&route, Shared<Kub3::HAL::Sensors::ISensor> sensor)
    {
        // Map the route in the router
        router->registerRoute(route, sensor);
        // Store sensor to member vector
        m_sensors.push_back(std::move(sensor));
    }

    // --- Motors HAL instanciation helpers

    Shared<Act::StepperMotor> HardwareManager::createStepperMotor(
        const Config::hardware_config_t &config,
        const std::string &motorId,
        uint8_t byteId,
        Algorithms::Kinematic::KinematicGeneratorKind kineGenKind,
        const Shared<MCUDriver> &driver,
        const std::string &encoderId)
    {
        auto it = config.motors.find(motorId);
        if (it == config.motors.end())
            throw std::runtime_error(std::format("Hardware configuration not found for key: '{}'", motorId));

        auto *hwProps = std::get_if<Config::stepper_hw_properties_t>(&it->second.hwProperties);
        if (!hwProps)
            throw std::runtime_error(std::format("'{}' configuration doesn't match expected type (stepper)", motorId));

        auto kinematicEngine = Algorithms::Kinematic::buildKinematicGenerator(kineGenKind);
        auto encoderGetter   = [repo = m_repo, encoderId]() { return HAL::MS::readInt(repo, encoderId); };

        return std::make_shared<Act::StepperMotor>(motorId, byteId, driver, *hwProps, std::move(encoderGetter), std::move(kinematicEngine));
    }

} // namespace Kub3::HAL

// Key extractors

static std::string_view arduino2KeyExtractor(const Kub3::HAL::Com::packet_t &packet)
{
    if (packet.length < 2)
        return std::string_view(); // Not enough bytes to have key + data

    const QByteArray &payload = packet.payload;

    switch (payload[0])
    {
    case 'C':
    {
        if (packet.length >= 3) // 'C1F' | 'C1B'
            return std::string_view(payload.data(), 3);
        break;
    }
    case 'E':
    {
        if (payload[1] == 'E') // Power off request
            return std::string_view(payload.data(), 2);
        else if (packet.length >= 3 && payload[1] == 0x7F && payload[2] == 0x7F) // Emergency stop
            return std::string_view(payload.data(), 3);
        break;
    }
    case 'I':
    {
        if (packet.length >= 3) // 'IT0' | 'IT1'
            return std::string_view(payload.data(), 3);
        break;
    }
    case 'K':
    {
        return std::string_view(payload.data(), 2); // 'K1' | 'K2' | 'K3' | 'K4'
    }
    case 'V':
    {
        if (packet.length < 3)
            break;
        if (packet.length >= 4 && payload[1] == 'V' && payload[2] == 'A' && payload[3] == 'C')
            return std::string_view(payload.data(), 4);
        return std::string_view(payload.data(), 3);
    }
    default:
        break;
    }
    return std::string_view(); // 404
}

static std::string_view arduino3KeyExtractor(const Kub3::HAL::Com::packet_t &packet)
{
    if (packet.length < 2)
        return std::string_view(); // Not enough bytes to have key + data

    const QByteArray &payload = packet.payload;

    switch (payload[0])
    {
    case 'S':
    {
        if (payload[1] == 'S' && packet.length >= 3)
            return std::string_view(payload.data(), 3);
        return std::string_view(payload.data(), 2);
    }
    case 'Z':
        return std::string_view(payload.data(), 2);
    case 'F':
    {
        if (payload[1] == '?' && packet.length >= 3)
            return std::string_view(payload.data(), 3);
        return std::string_view(payload.data(), 2);
    }
    default:
    {
        if ('1' <= payload[0] && payload[0] <= '5')
            return std::string_view(payload.data(), 1);
        break;
    }
    }
    return std::string_view(); // 404
}

// Parsers

static bool limitSwitchParser(const QByteArray &d)
{
    return !d.isEmpty() && d[0] != 0x0;
}

static bool valveStatusParser(const QByteArray &d)
{
    return !d.isEmpty() && d[0] != '0';
}

static bool pressureSensorParser(const QByteArray &d)
{
    return !d.isEmpty() && d[0] != '0';
}

static int32_t temperatureParser(const QByteArray &d)
{
    if (d.size() < 2) // Not enough data to read
        return INT32_MIN;
    // Big-endian reconstruction
    return (static_cast<int32_t>(static_cast<uint8_t>(d[0])) << 8) |
           (static_cast<int32_t>(static_cast<uint8_t>(d[1])));
}

static uint16_t forceSensorParser(const QByteArray &d)
{
    if (d.size() < 4) // Not enough data to read
        return (uint16_t)0u;

    uint16_t result = 0;

    for (uint8_t idx = 0; idx < 4; ++idx)
        result = (result * 10) + (d[idx] - '0');
    return result;
}

static bool forceSensorEnabledParser(const QByteArray &d)
{
    if (d.isEmpty())
        return false;

    return d[0] == '1';
}

static int32_t encoderValueParser(const QByteArray &d)
{
    if (d.size() < 4) // Not enough data to read
        return 0;
    // Big-endian reconstruction
    return (static_cast<int32_t>(static_cast<uint8_t>(d[0])) << 24) |
           (static_cast<int32_t>(static_cast<uint8_t>(d[1])) << 16) |
           (static_cast<int32_t>(static_cast<uint8_t>(d[2])) << 8) |
           (static_cast<int32_t>(static_cast<uint8_t>(d[3])));
}

static bool physicalButtonParser(const QByteArray &d)
{
    return true; // @note: Receiving message only when button was pressed
}
