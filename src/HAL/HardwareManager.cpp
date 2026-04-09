#include <QDebug>

#include "Algorithms/Kinematic/IKinematicGenerator.h"
#include "HAL/Com/LengthBasedParser.h"
#include "HAL/Com/SerialCommunicator.h"
#include "HAL/HardwareManager.h"
#include "HAL/MachineStatus/actuators_labels.h"
#include "HAL/MachineStatus/sensors_labels.h"
#include "HAL/MachineStatus/utils.h"
#include "HAL/Sensors/Sensor.h"

using namespace std::string_literals;

// Packet key extractors
static std::string_view arduino2KeyExtractor(const Kub3::HAL::Com::packet_t &packet);
static std::string_view arduino3KeyExtractor(const Kub3::HAL::Com::packet_t &packet);
// Payload parsers
static bool limitSwitchParser(const QByteArray &d);
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
        for (size_t i = 0; i < m_threads.size(); ++i)
        {
            m_threads[i]->start();
            QMetaObject::invokeMethod(m_drivers[i].get(), &MCUDriver::ps_start);
        }
    }

    void HardwareManager::stopAll()
    {
        for (size_t i = 0; i < m_threads.size(); ++i)
        {
            if (!m_threads[i] || !m_threads[i]->isRunning())
                continue;

            QMetaObject::invokeMethod(m_drivers[i].get(), &MCUDriver::ps_stop, Qt::QueuedConnection);
            m_threads[i]->exit(0);
            if (!m_threads[i]->wait(2000))
            {
                m_threads[i]->terminate(); // Force kill
                m_threads[i]->wait();
            }
        }
    }

#if defined(KUB_MODEL_8)

    // ======================================================
    // KUB3-8i HAL instanciation functions (split by MCU)
    // ======================================================

    void HardwareManager::setupArduino2Subsystem(const Config::hardware_config_t &config)
    {
        using namespace Kub3::HAL::Sensors;

        // Create thread & Driver for Arduino3
        auto thread         = std::make_unique<QThread>();                                       // Driver thread
        auto comms          = std::make_unique<Com::SerialCommunicator>("/dev/ttyACM1", 115200); // TODO: get port from settings file
        auto parser         = std::make_unique<Com::LengthBasedParser>();
        auto arduino2Driver = std::make_shared<MCUDriver>(std::move(comms), std::move(parser));
        auto router         = std::make_unique<Com::PacketRouter>(&arduino2KeyExtractor);

        // ===========================================
        // UPWARD PIPELINE (Hardware --> Software)
        // ===========================================

        // Create Sensors
        // --- Physical buttons
        auto emergencyStopBtn = std::make_shared<Sensor<bool>>(m_repo, EMERGENCY_STOP, false, &physicalButtonParser);

        // Register sensors
        // --- Physical buttons
        this->registerSensor(router.get(), "E\x7F\x7F"s, std::move(emergencyStopBtn));

        // Move MCUDriver to its own thread
        arduino2Driver->moveToThread(thread.get());

        // Wire MCUDriver -> Router
        QObject::connect(arduino2Driver.get(), &MCUDriver::s_packetReady, router.get(), &Com::PacketRouter::ps_routePacket);

        // Store in lifecycle manager
        m_drivers.push_back(std::move(arduino2Driver));
        m_routers.push_back(std::move(router));
        m_threads.push_back(std::move(thread));
    }

    void HardwareManager::setupArduino3Subsystem(const Config::hardware_config_t &config)
    {
        // Create thread & Driver for Arduino3
        auto thread         = std::make_unique<QThread>();                                       // Driver thread
        auto comms          = std::make_unique<Com::SerialCommunicator>("/dev/ttyACM0", 115200); // TODO: get port from settings file
        auto parser         = std::make_unique<Com::LengthBasedParser>();
        auto arduino3Driver = std::make_shared<MCUDriver>(std::move(comms), std::move(parser));
        auto router         = std::make_unique<Com::PacketRouter>(&arduino3KeyExtractor);

        this->createArduino3Sensors(router.get());
        this->createArduino3Actuators(config, arduino3Driver);

        // Move MCUDriver to its own thread
        arduino3Driver->moveToThread(thread.get());

        // Wire MCUDriver -> Router
        QObject::connect(arduino3Driver.get(), &MCUDriver::s_packetReady, router.get(), &Com::PacketRouter::ps_routePacket);

        // Store in lifecycle manager
        m_drivers.push_back(std::move(arduino3Driver));
        m_routers.push_back(std::move(router));
        m_threads.push_back(std::move(thread));
    }

    // ========================================
    // Arduino 3 HAL instanciation helpers
    // ========================================

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

    // =====================================
    // Sensors HAL instanciation helpers
    // =====================================

    void HardwareManager::registerSensor(Com::PacketRouter *router, std::string &&route, Shared<Kub3::HAL::Sensors::ISensor> sensor)
    {
        // Map the route in the router
        router->registerRoute(route, sensor);
        // Store sensor to member vector
        m_sensors.push_back(std::move(sensor));
    }

    // =====================================
    // Motors HAL instanciation helpers
    // =====================================

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

    // TODO: implement more keys
    switch (packet.payload[0])
    {
    case 'E':
    {
        if (packet.payload[1] == 'E') // Shutdown request
            return std::string_view(packet.payload.data(), 2);
        else if (packet.length >= 3 && packet.payload[1] == 0x7F && packet.payload[2] == 0x7F) // Emergency stop
            return std::string_view(packet.payload.data(), 3);
        break;
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

    switch (packet.payload[0])
    {
    case 'S':
    {
        if (packet.payload[1] == 'S' && packet.length >= 3)
            return std::string_view(packet.payload.data(), 3);
        return std::string_view(packet.payload.data(), 2);
    }
    case 'Z':
        return std::string_view(packet.payload.data(), 2);
    case 'F':
    {
        if (packet.payload[1] == '?' && packet.length >= 3)
            return std::string_view(packet.payload.data(), 3);
        return std::string_view(packet.payload.data(), 2);
    }
    default:
    {
        if ('1' <= packet.payload[0] && packet.payload[0] <= '5')
            return std::string_view(packet.payload.data(), 1);
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
