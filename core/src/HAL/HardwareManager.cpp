#include <QCoreApplication>
#include <QDebug>
#include <QRect>
#include <memory>

#include <Algorithms/Kinematic/IKinematicGenerator.h>
#include <Config/hardware.h>
#include <Config/kinematics.h>
#include <HAL/Actuators/Focal/Focal.h>
#include <HAL/Actuators/Lights/CameraLightingLed.h>
#include <HAL/Actuators/Lights/UVExposureHead.h>
#include <HAL/Actuators/Motors/DirectCurrentMotor.h>
#include <HAL/Actuators/Switches/LogicSwitch.h>
#include <HAL/Actuators/Valves/SolenoidValve.h>
#include <HAL/Com/LengthBasedParser.h>
#include <HAL/Com/MCULogger.h>
#include <HAL/Com/SerialCommunicator.h>
#include <HAL/HardwareManager.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/Sensors/KinematicEncoderSensor.h>
#include <HAL/Sensors/Sensor.h>
#include <HAL/Vision/Hikrobot/HikrobotCamera.h>

using namespace std::string_literals;

#define ARDUINO1_INDEX 0
#define ARDUINO2_INDEX 1
#define ARDUINO3_INDEX 2
#define ARDUINO4_INDEX 3

// Packet key extractors
static std::string_view arduino1KeyExtractor(const Kub3::HAL::Com::packet_t &packet);
static std::string_view arduino2KeyExtractor(const Kub3::HAL::Com::packet_t &packet);
static std::string_view arduino3KeyExtractor(const Kub3::HAL::Com::packet_t &packet);
// Payload parsers
static bool limitSwitchParser(const QByteArray &d);
static bool valveStatusParser(const QByteArray &d);
static bool pressureSensorParser(const QByteArray &d);
static double temperatureParser(const QByteArray &d);
static uint16_t forceSensorParser(const QByteArray &d);
static bool forceSensorEnabledParser(const QByteArray &d);
static int32_t encoderValueParser(const QByteArray &d);
static uint16_t fansVoltageParser(const QByteArray &d);
static uint32_t ledLineVoltageParser(const QByteArray &d);
static bool physicalButtonParser(const QByteArray &d);
// Helpers
static double getPositionMotorEncoderConversionFactor(const QString &motorId, const Kub3::Config::hardware_config_t &config);

namespace Kub3::HAL
{

    HardwareManager::HardwareManager(Shared<MS::IMachineStatusRepo> repo, const Config::hardware_config_t &config, QObject *parent) :
        QObject(parent),
        m_repo(std::move(repo)),
        m_actuatorRegistry(std::make_shared<Act::ActuatorRegistry>())
    {
        m_mcusLoggerThread = new QThread(this);
        m_mcusLoggerThread->setObjectName("MCUsSharedLoggerThread");
        m_mcusLoggerThread->start(QThread::LowPriority);

// TODO: Build the machine based on the CMake configuration
#if defined(KUB_MODEL_8)
        setupArduino1Subsystem(config);
        setupArduino2Subsystem(config);
        setupArduino3Subsystem(config);
        setupCamerasSubsystem(config);
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
            QMetaObject::invokeMethod(subsys.driver.get(), &MCUDriver::ps_start, Qt::QueuedConnection);
        }

        for (auto &[key, subsys] : m_cameras)
        {
            subsys.thread->start();
            QMetaObject::invokeMethod(subsys.camera.get(), &Vision::ICamera::connectDevice, Qt::QueuedConnection);
            QMetaObject::invokeMethod(subsys.camera.get(), &Vision::ICamera::startAcquisition, Qt::QueuedConnection);
        }
    }

    void HardwareManager::stopAll()
    {
        for (auto &[key, subsys] : m_subsystems)
        {
            if (subsys.logger)
            {
                QMetaObject::invokeMethod(subsys.logger, &Com::MCULogger::flush, Qt::BlockingQueuedConnection);
                subsys.logger->deleteLater();
                subsys.logger = nullptr;
            }
        }

        if (m_mcusLoggerThread && m_mcusLoggerThread->isRunning())
        {
            m_mcusLoggerThread->quit();
            m_mcusLoggerThread->wait();
        }

        auto stopThreadHelper = [this](QThread *thread, std::function<void(void)> stopMethod) {
            if (!thread || !thread->isRunning())
                return;

            if (stopMethod)
                stopMethod();
            thread->exit(0);
            if (!thread->wait(2000))
            {
                thread->terminate(); // Force kill
                thread->wait();
            }
        };

        for (auto &[key, subsys] : m_subsystems)
        {
            stopThreadHelper(
                subsys.thread.get(),
                [&subsys]() {
                    QMetaObject::invokeMethod(subsys.driver.get(), &MCUDriver::ps_stop, Qt::BlockingQueuedConnection);
                });
        }

        for (auto &[key, subsys] : m_cameras)
        {
            stopThreadHelper(
                subsys.thread.get(),
                [&subsys]() {
                    QMetaObject::invokeMethod(subsys.camera.get(), &Vision::ICamera::disconnectDevice, Qt::BlockingQueuedConnection);
                });
        }
    }

    void HardwareManager::ps_reconnectMCUSubsystem(const QString &subsystemId)
    {
        if (auto it = m_subsystems.find(subsystemId); it != m_subsystems.end())
        {
            auto &subsys = it->second;

            qInfo() << "HardwareManager: Reconnecting targeted subsystem:" << subsystemId;
            if (!subsys.thread)
            {
                qCritical() << "HardwareManager: Attempt to reconnect using invalid thread pointer:" << subsystemId;
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
            qWarning() << "HardwareManager: Requested restart for unknown subsystem:" << subsystemId;
        }
    }

    void HardwareManager::ps_reconnectCameraSubsystem(const QString &cameraId)
    {
        if (auto it = m_cameras.find(cameraId); it != m_cameras.end())
        {
            auto &node = it->second;

            qInfo() << "HardwareManager: Bouncing camera subsystem:" << cameraId;
            // Push stop/start commands to the camera thread queue
            QMetaObject::invokeMethod(node.camera.get(), &Vision::ICamera::stopAcquisition, Qt::QueuedConnection);
            QMetaObject::invokeMethod(node.camera.get(), &Vision::ICamera::disconnectDevice, Qt::QueuedConnection);
            QMetaObject::invokeMethod(node.camera.get(), &Vision::ICamera::connectDevice, Qt::QueuedConnection);
            QMetaObject::invokeMethod(node.camera.get(), &Vision::ICamera::startAcquisition, Qt::QueuedConnection);
        }
    }

    void HardwareManager::ps_updateCameraParameter(const QString &cameraId, Vision::CameraParamKind kind, Vision::CameraParam value)
    {
        if (auto it = m_cameras.find(cameraId); it != m_cameras.end())
        {
            switch (kind)
            {
            case Vision::CameraParamKind::EXPOSURE:
            {
                if (const double *val = std::get_if<double>(&value))
                    it->second.camera->setExposure(*val);
                break;
            }
            case Vision::CameraParamKind::GAIN:
            {
                if (const double *val = std::get_if<double>(&value))
                    it->second.camera->setGain(*val);
                break;
            }
            case Vision::CameraParamKind::FRAMERATE:
            {
                if (const double *val = std::get_if<double>(&value))
                    it->second.camera->setFrameRate(*val);
                break;
            }
            case Vision::CameraParamKind::CENTERED_ZOOM:
            {
                if (const double *val = std::get_if<double>(&value))
                    it->second.camera->setCenteredZoom(*val);
                break;
            }
            case Vision::CameraParamKind::REGION_OF_INTEREST:
            {
                if (const QRect *val = std::get_if<QRect>(&value))
                    it->second.camera->setROI(val->x(), val->y(), val->width(), val->height());
                break;
            }
            default:
                break;
            }
        }
    }

    void HardwareManager::ps_powerOff(void)
    {
        if (auto it = m_subsystems.find(MCU_ARDUINO2_ID); it != m_subsystems.end())
        {
            it->second.driver->sendCommand(QByteArray(1, 'E'), Qt::BlockingQueuedConnection);
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

    // --- Arduino 1 HAL instanciation helpers

    void HardwareManager::setupArduino1Subsystem(const Config::hardware_config_t &config)
    {
        using namespace Kub3::HAL::Sensors;

        // Create thread & Driver for Arduino3
        auto mcuConf        = config.mcus[ARDUINO1_INDEX];
        auto thread         = std::make_unique<QThread>(); // Driver thread
        auto comms          = std::make_unique<Com::SerialCommunicator>(mcuConf.port, mcuConf.baudrate);
        auto parser         = std::make_unique<Com::LengthBasedParser>();
        auto arduino1Driver = std::make_shared<MCUDriver>(std::move(comms), std::move(parser));
        auto router         = std::make_unique<Com::PacketRouter>(&arduino1KeyExtractor);

        // Initialize MCU driver connection "sensor". Doesn't need a parser.
        m_sensors.push_back(std::make_shared<HAL::Sensors::Sensor<bool>>(m_repo, MCU_ARDUINO1_READY, false, nullptr));

        // Instanciate sensors and actuators software representations
        this->createArduino1Sensors(config, router.get());
        this->createArduino1Actuators(config, arduino1Driver, router.get());

        // Inject logger
        auto logger = new Com::MCULogger("arduino1", arduino1Driver.get());

        // Move logger to the logger thread
        logger->moveToThread(m_mcusLoggerThread);
        // Move MCUDriver to its own thread
        arduino1Driver->moveToThread(thread.get());

        // Wire MCUDriver connection status signals -> Machine Status Repo value update
        QObject::connect(arduino1Driver.get(), &MCUDriver::s_connected, [&, driver = arduino1Driver]() {
            m_repo->setValueRaw(MCU_ARDUINO1_READY, true);
            driver->sendCommand(QByteArray("?S"));
            driver->sendCommand(QByteArray("?C"));
        });
        QObject::connect(arduino1Driver.get(), &MCUDriver::s_connectionLost, [&]() { m_repo->setValueRaw(MCU_ARDUINO1_READY, false); });
        // Wire MCUDriver -> Router
        QObject::connect(arduino1Driver.get(), &MCUDriver::s_packetReady, router.get(), &Com::PacketRouter::ps_routePacket);

        // Store in lifecycle manager
        m_subsystems[MCU_ARDUINO1_ID] = MCUSubsystemNode{
            .thread = std::move(thread),
            .driver = std::move(arduino1Driver),
            .router = std::move(router),
            .logger = logger,
        };
    }

    void HardwareManager::createArduino1Sensors(const Kub3::Config::hardware_config_t &config, Com::PacketRouter *router)
    {
        using namespace Kub3::HAL::Sensors;

        const auto getEncConvFactor = &getPositionMotorEncoderConversionFactor;

        // ===========================================
        // UPWARD PIPELINE (Hardware --> Software)
        // ===========================================

        // Create Sensors
        // --- Limit switches
        auto leftCameraXLeftLimit         = std::make_shared<Sensor<bool>>(m_repo, LEFT_CAMERA_X_LEFT_LIMIT, false, &limitSwitchParser);
        auto leftCameraXRightLimit        = std::make_shared<Sensor<bool>>(m_repo, LEFT_CAMERA_X_RIGHT_LIMIT, false, &limitSwitchParser);
        auto leftCameraYFrontLimit        = std::make_shared<Sensor<bool>>(m_repo, LEFT_CAMERA_Y_FRONT_LIMIT, false, &limitSwitchParser);
        auto leftCameraYBackLimit         = std::make_shared<Sensor<bool>>(m_repo, LEFT_CAMERA_Y_BACK_LIMIT, false, &limitSwitchParser);
        auto rightCameraXLeftLimit        = std::make_shared<Sensor<bool>>(m_repo, RIGHT_CAMERA_X_LEFT_LIMIT, false, &limitSwitchParser);
        auto rightCameraXRightLimit       = std::make_shared<Sensor<bool>>(m_repo, RIGHT_CAMERA_X_RIGHT_LIMIT, false, &limitSwitchParser);
        auto rightCameraYFrontLimit       = std::make_shared<Sensor<bool>>(m_repo, RIGHT_CAMERA_Y_FRONT_LIMIT, false, &limitSwitchParser);
        auto rightCameraYBackLimit        = std::make_shared<Sensor<bool>>(m_repo, RIGHT_CAMERA_Y_BACK_LIMIT, false, &limitSwitchParser);
        auto xStageLeftLimit              = std::make_shared<Sensor<bool>>(m_repo, X_STAGE_LEFT_LIMIT, false, &limitSwitchParser);
        auto xStageRightLimit             = std::make_shared<Sensor<bool>>(m_repo, X_STAGE_RIGHT_LIMIT, false, &limitSwitchParser);
        auto yStageFrontLimit             = std::make_shared<Sensor<bool>>(m_repo, Y_STAGE_FRONT_LIMIT, false, &limitSwitchParser);
        auto yStageBackLimit              = std::make_shared<Sensor<bool>>(m_repo, Y_STAGE_BACK_LIMIT, false, &limitSwitchParser);
        auto thetaStageClockwiseLimit     = std::make_shared<Sensor<bool>>(m_repo, THETA_STAGE_CLOCKWISE_LIMIT, false, &limitSwitchParser);
        auto thetaStageAntiClockwiseLimit = std::make_shared<Sensor<bool>>(m_repo, THETA_STAGE_ANTI_CLOCKWISE_LIMIT, false, &limitSwitchParser);
        // --- Encoders
        auto leftCameraXEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, LEFT_CAMERA_X_ENCODER, LEFT_CAMERA_X_ENCODER_MM, &encoderValueParser, getEncConvFactor(LEFT_CAMERA_X_MOTOR, config));
        auto leftCameraYEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, LEFT_CAMERA_Y_ENCODER, LEFT_CAMERA_Y_ENCODER_MM, &encoderValueParser, getEncConvFactor(LEFT_CAMERA_Y_MOTOR, config));
        auto rightCameraXEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, RIGHT_CAMERA_X_ENCODER, RIGHT_CAMERA_X_ENCODER_MM, &encoderValueParser, getEncConvFactor(RIGHT_CAMERA_X_MOTOR, config));
        auto rightCameraYEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, RIGHT_CAMERA_Y_ENCODER, RIGHT_CAMERA_Y_ENCODER_MM, &encoderValueParser, getEncConvFactor(RIGHT_CAMERA_Y_MOTOR, config));
        auto xStageEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, X_STAGE_ENCODER, X_STAGE_ENCODER_MM, &encoderValueParser, getEncConvFactor(X_STAGE_MOTOR, config));
        auto yStageEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, Y_STAGE_ENCODER, Y_STAGE_ENCODER_MM, &encoderValueParser, getEncConvFactor(Y_STAGE_MOTOR, config));
        auto thetaStageEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, THETA_STAGE_ENCODER, THETA_STAGE_ENCODER_MM, &encoderValueParser, getEncConvFactor(THETA_STAGE_MOTOR, config));

        // Register sensors
        // --- Limit switches
        this->registerSensor(router, "S\x00"s, std::move(leftCameraXLeftLimit));
        this->registerSensor(router, "S\x01"s, std::move(leftCameraYFrontLimit));
        this->registerSensor(router, "S\x02"s, std::move(rightCameraXRightLimit));
        this->registerSensor(router, "S\x03"s, std::move(rightCameraYFrontLimit));
        // this->registerSensor(router, "S\x01"s, std::move(leftCameraXRightLimit));
        // this->registerSensor(router, "S\x03"s, std::move(leftCameraYBackLimit));
        // this->registerSensor(router, "S\x05"s, std::move(rightCameraXLeftLimit));
        // this->registerSensor(router, "S\x07"s, std::move(rightCameraYBackLimit));
        this->registerSensor(router, "S\x04"s, std::move(xStageRightLimit));
        this->registerSensor(router, "S\x05"s, std::move(yStageFrontLimit));
        this->registerSensor(router, "S\x07"s, std::move(thetaStageAntiClockwiseLimit));
        this->registerSensor(router, "S\x08"s, std::move(xStageLeftLimit));
        this->registerSensor(router, "S\x09"s, std::move(yStageBackLimit));
        this->registerSensor(router, "S\x0B"s, std::move(thetaStageClockwiseLimit));
        // --- Encoders
        this->registerSensor(router, "1"s, std::move(leftCameraXEncoder));
        this->registerSensor(router, "2"s, std::move(leftCameraYEncoder));
        this->registerSensor(router, "3"s, std::move(rightCameraXEncoder));
        this->registerSensor(router, "4"s, std::move(rightCameraYEncoder));
        this->registerSensor(router, "5"s, std::move(xStageEncoder));
        this->registerSensor(router, "6"s, std::move(yStageEncoder));
        this->registerSensor(router, "8"s, std::move(thetaStageEncoder));
    }

    void HardwareManager::createArduino1Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver, Com::PacketRouter *router)
    {
        // ===========================================
        // DOWNWARD PIPELINE (Software --> Hardware)
        // ===========================================

        using Kinematics = Algorithms::Kinematic::KinematicGeneratorKind;

        /// --- Motors
        auto leftCameraXMotor  = createStepperMotor(config, LEFT_CAMERA_X_MOTOR, '1', Kinematics::TRAPEZOIDAL, driver, LEFT_CAMERA_X_ENCODER);
        auto leftCameraYMotor  = createStepperMotor(config, LEFT_CAMERA_Y_MOTOR, '2', Kinematics::TRAPEZOIDAL, driver, LEFT_CAMERA_Y_ENCODER);
        auto rightCameraXMotor = createStepperMotor(config, RIGHT_CAMERA_X_MOTOR, '3', Kinematics::TRAPEZOIDAL, driver, RIGHT_CAMERA_X_ENCODER);
        auto rightCameraYMotor = createStepperMotor(config, RIGHT_CAMERA_Y_MOTOR, '4', Kinematics::TRAPEZOIDAL, driver, RIGHT_CAMERA_Y_ENCODER);
        auto xStageMotor       = createStepperMotor(config, X_STAGE_MOTOR, '5', Kinematics::TRAPEZOIDAL, driver, X_STAGE_ENCODER);
        auto yStageMotor       = createStepperMotor(config, Y_STAGE_MOTOR, '6', Kinematics::TRAPEZOIDAL, driver, Y_STAGE_ENCODER);
        auto thetaStageMotor   = createStepperMotor(config, THETA_STAGE_MOTOR, '8', Kinematics::TRAPEZOIDAL, driver, THETA_STAGE_ENCODER);
        // TODO: Replace magic value `4095` with config based value
        /// --- Focals
        auto leftCameraFocal  = std::make_shared<Act::Focal>(LEFT_CAMERA_FOCAL, 'L', 4095, driver);
        auto rightCameraFocal = std::make_shared<Act::Focal>(RIGHT_CAMERA_FOCAL, 'R', 4095, driver);
        /// --- Cameras lighting leds
        auto leftCameraLight  = std::make_shared<Act::CameraLightingLed>(LEFT_CAMERA_LIGHT, 'L', 4095, driver);
        auto rightCameraLight = std::make_shared<Act::CameraLightingLed>(RIGHT_CAMERA_LIGHT, 'R', 4095, driver);

        /* FEEDBACK HANDLING */
        router->registerRoute("SS\x00"s, Act::StepperMotor::createFeedbackHandler(leftCameraXMotor));
        router->registerRoute("SS\x01"s, Act::StepperMotor::createFeedbackHandler(leftCameraYMotor));
        router->registerRoute("SS\x02"s, Act::StepperMotor::createFeedbackHandler(rightCameraXMotor));
        router->registerRoute("SS\x03"s, Act::StepperMotor::createFeedbackHandler(rightCameraYMotor));
        router->registerRoute("SS\x04"s, Act::StepperMotor::createFeedbackHandler(xStageMotor));
        router->registerRoute("SS\x05"s, Act::StepperMotor::createFeedbackHandler(yStageMotor));
        router->registerRoute("SS\x07"s, Act::StepperMotor::createFeedbackHandler(thetaStageMotor));

        /// --- Motors
        m_actuatorRegistry->registerActuator(std::move(leftCameraXMotor));
        m_actuatorRegistry->registerActuator(std::move(leftCameraYMotor));
        m_actuatorRegistry->registerActuator(std::move(rightCameraXMotor));
        m_actuatorRegistry->registerActuator(std::move(rightCameraYMotor));
        m_actuatorRegistry->registerActuator(std::move(xStageMotor));
        m_actuatorRegistry->registerActuator(std::move(yStageMotor));
        m_actuatorRegistry->registerActuator(std::move(thetaStageMotor));
        /// --- Focals
        m_actuatorRegistry->registerActuator(std::move(leftCameraFocal));
        m_actuatorRegistry->registerActuator(std::move(rightCameraFocal));
        m_registeredFocalIds.push_back(LEFT_CAMERA_FOCAL);
        m_registeredFocalIds.push_back(RIGHT_CAMERA_FOCAL);
        /// --- Cameras lighting leds
        m_actuatorRegistry->registerActuator(std::move(leftCameraLight));
        m_actuatorRegistry->registerActuator(std::move(rightCameraLight));
        m_registeredLightIds.push_back(LEFT_CAMERA_LIGHT);
        m_registeredLightIds.push_back(RIGHT_CAMERA_LIGHT);
    }

    // --- Arduino 2 HAL instanciation helpers

    void HardwareManager::setupArduino2Subsystem(const Config::hardware_config_t &config)
    {
        using namespace Kub3::HAL::Sensors;

        // Create thread & Driver for Arduino3
        auto mcuConf        = config.mcus[ARDUINO2_INDEX];
        auto thread         = std::make_unique<QThread>(); // Driver thread
        auto comms          = std::make_unique<Com::SerialCommunicator>(mcuConf.port, mcuConf.baudrate);
        auto parser         = std::make_unique<Com::LengthBasedParser>();
        auto arduino2Driver = std::make_shared<MCUDriver>(std::move(comms), std::move(parser));
        auto router         = std::make_unique<Com::PacketRouter>(&arduino2KeyExtractor);

        // Initialize MCU driver connection "sensor". Doesn't need a parser.
        m_sensors.push_back(std::make_shared<HAL::Sensors::Sensor<bool>>(m_repo, MCU_ARDUINO2_READY, false, nullptr));

        // Instanciate sensors and actuators software representations
        this->createArduino2Sensors(config, router.get());
        this->createArduino2Actuators(config, arduino2Driver, router.get());

        // Inject logger
        auto logger = new Com::MCULogger("arduino2", arduino2Driver.get());

        // Move logger to the logger thread
        logger->moveToThread(m_mcusLoggerThread);
        // Move MCUDriver to its own thread
        arduino2Driver->moveToThread(thread.get());

        // Wire MCUDriver connection status signals -> Machine Status Repo value update
        QObject::connect(arduino2Driver.get(), &MCUDriver::s_connected, [&, driver = arduino2Driver]() {
            m_repo->setValueRaw(MCU_ARDUINO2_READY, true);
            driver->sendCommand(QByteArray("?C"));
            driver->sendCommand(QByteArray("?K"));
        });
        QObject::connect(arduino2Driver.get(), &MCUDriver::s_connectionLost, [&]() { m_repo->setValueRaw(MCU_ARDUINO2_READY, false); });
        // Wire MCUDriver -> Router
        QObject::connect(arduino2Driver.get(), &MCUDriver::s_packetReady, router.get(), &Com::PacketRouter::ps_routePacket);

        // Store in lifecycle manager
        m_subsystems[MCU_ARDUINO2_ID] = MCUSubsystemNode{
            .thread = std::move(thread),
            .driver = std::move(arduino2Driver),
            .router = std::move(router),
            .logger = logger,
        };
    }

    void HardwareManager::createArduino2Sensors(const Kub3::Config::hardware_config_t &config, Com::PacketRouter *router)
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
        auto ardkoFrontLeftLimit   = std::make_shared<Sensor<bool>>(m_repo, ARDKO_FRONT_LEFT_LIMIT, false, &limitSwitchParser);
        auto ardkoFrontRightLimit  = std::make_shared<Sensor<bool>>(m_repo, ARDKO_FRONT_RIGHT_LIMIT, false, &limitSwitchParser);
        auto ardkoBackLeftLimit    = std::make_shared<Sensor<bool>>(m_repo, ARDKO_BACK_LEFT_LIMIT, false, &limitSwitchParser);
        auto ardkoBackRightLimit   = std::make_shared<Sensor<bool>>(m_repo, ARDKO_BACK_RIGHT_LIMIT, false, &limitSwitchParser);
        // --- Solenoid valves statii
        auto maskVacuumValveStatus         = std::make_shared<Sensor<bool>>(m_repo, MASK_VACUUM_VALVE_STATUS, false, &valveStatusParser);
        auto waferVacuumValveStatus        = std::make_shared<Sensor<bool>>(m_repo, WAFER_VACUUM_VALVE_STATUS, false, &valveStatusParser);
        auto waferCompressedAirValveStatus = std::make_shared<Sensor<bool>>(m_repo, WAFER_COMPRESSED_AIR_VALVE_STATUS, false, &valveStatusParser);
        // --- Pressure sensors
        auto maskVacuumActive         = std::make_shared<Sensor<bool>>(m_repo, MASK_VACUUM_ACTIVE, false, &pressureSensorParser);
        auto waferVacuumActive        = std::make_shared<Sensor<bool>>(m_repo, WAFER_VACUUM_ACTIVE, false, &pressureSensorParser);
        auto waferCompressedAirActive = std::make_shared<Sensor<bool>>(m_repo, WAFER_COMPRESSED_AIR_ACTIVE, false, &pressureSensorParser);
        // --- Temperatures
        auto internalTemperature = std::make_shared<Sensor<double>>(m_repo, INTERNAL_TEMPERATURE, -100.0, &temperatureParser);
        auto externalTemperature = std::make_shared<Sensor<double>>(m_repo, EXTERNAL_TEMPERATURE, -100.0, &temperatureParser);
        // --- Encoders
        // auto camerasDeckEncoder = std::make_shared<Sensor<int32_t>>(m_repo, DECK_MOTOR_ENCODER, static_cast<int32_t>(0), &encoderValueParser);
        // --- Fans voltage
        auto fansVoltage = std::make_shared<Sensor<uint16_t>>(m_repo, FANS_VOLTAGE, static_cast<uint16_t>(0), &fansVoltageParser);
        // --- Leds voltage
        // TODO: Modify either MCU communication protocol or the Sensor<T> variant to be able to split the values in two distinct Sensor<uint16_t>
        auto innerLedLineVoltage1 = std::make_shared<Sensor<uint32_t>>(m_repo, INNER_LED_LINE_VOLTAGES(0), static_cast<uint32_t>(0), &ledLineVoltageParser);
        auto innerLedLineVoltage2 = std::make_shared<Sensor<uint32_t>>(m_repo, INNER_LED_LINE_VOLTAGES(1), static_cast<uint32_t>(0), &ledLineVoltageParser);
        auto innerLedLineVoltage3 = std::make_shared<Sensor<uint32_t>>(m_repo, INNER_LED_LINE_VOLTAGES(2), static_cast<uint32_t>(0), &ledLineVoltageParser);
        auto innerLedLineVoltage4 = std::make_shared<Sensor<uint32_t>>(m_repo, INNER_LED_LINE_VOLTAGES(3), static_cast<uint32_t>(0), &ledLineVoltageParser);
        auto outerLedLineVoltage1 = std::make_shared<Sensor<uint32_t>>(m_repo, OUTER_LED_LINE_VOLTAGES(0), static_cast<uint32_t>(0), &ledLineVoltageParser);
        auto outerLedLineVoltage2 = std::make_shared<Sensor<uint32_t>>(m_repo, OUTER_LED_LINE_VOLTAGES(1), static_cast<uint32_t>(0), &ledLineVoltageParser);
        auto outerLedLineVoltage3 = std::make_shared<Sensor<uint32_t>>(m_repo, OUTER_LED_LINE_VOLTAGES(2), static_cast<uint32_t>(0), &ledLineVoltageParser);
        auto outerLedLineVoltage4 = std::make_shared<Sensor<uint32_t>>(m_repo, OUTER_LED_LINE_VOLTAGES(3), static_cast<uint32_t>(0), &ledLineVoltageParser);

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
        // --- Fans voltage
        this->registerSensor(router, "IF"s, std::move(fansVoltage));
        // --- Internal leds voltage
        this->registerSensor(router, "IVI\x00"s, std::move(innerLedLineVoltage1));
        this->registerSensor(router, "IVI\x01"s, std::move(innerLedLineVoltage2));
        this->registerSensor(router, "IVI\x02"s, std::move(innerLedLineVoltage3));
        this->registerSensor(router, "IVI\x03"s, std::move(innerLedLineVoltage4));
        // --- External leds voltage
        this->registerSensor(router, "IVC\x00"s, std::move(outerLedLineVoltage1));
        this->registerSensor(router, "IVC\x01"s, std::move(outerLedLineVoltage2));
        this->registerSensor(router, "IVC\x02"s, std::move(outerLedLineVoltage3));
        this->registerSensor(router, "IVC\x03"s, std::move(outerLedLineVoltage4));
    }

    void HardwareManager::createArduino2Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver, Com::PacketRouter *router)
    {
        using Kinematics = Algorithms::Kinematic::KinematicGeneratorKind;

        // ===========================================
        // DOWNWARD PIPELINE (Software --> Hardware)
        // ===========================================

        /// --- Motors
        auto camerasDeckMotor = createDCMotor(config, DECK_MOTOR, 'F', Kinematics::TRAPEZOIDAL, driver);
        /// --- Valves
        auto maskVacuumValve         = std::make_shared<Act::SolenoidValve>(MASK_VACUUM_VALVE, "VEM14095", "VEM00", driver);
        auto waferVacuumValve        = std::make_shared<Act::SolenoidValve>(WAFER_VACUUM_VALVE, "VEW14095", "VEW00", driver);
        auto waferCompressedAirValve = std::make_shared<Act::SolenoidValve>(WAFER_COMPRESSED_AIR_VALVE, "AC1", "AC0", driver);
        /// --- Exposure head
        auto exposureHead = std::make_shared<Act::UVExposureHead>(UV_EXPOSURE_HEAD, driver);

        router->registerRoute("CL1", Act::DirectCurrentMotor::createFeedbackHandler(camerasDeckMotor));

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
        auto mcuConf        = config.mcus[ARDUINO3_INDEX];
        auto thread         = std::make_unique<QThread>(); // Driver thread
        auto comms          = std::make_unique<Com::SerialCommunicator>(mcuConf.port, mcuConf.baudrate);
        auto parser         = std::make_unique<Com::LengthBasedParser>();
        auto arduino3Driver = std::make_shared<MCUDriver>(std::move(comms), std::move(parser));
        auto router         = std::make_unique<Com::PacketRouter>(&arduino3KeyExtractor);

        // Initialize MCU driver connection "sensor". Doesn't need a parser.
        m_sensors.push_back(std::make_shared<HAL::Sensors::Sensor<bool>>(m_repo, MCU_ARDUINO3_READY, false, nullptr));

        // Instanciate sensors and actuators software representations
        this->createArduino3Sensors(config, router.get());
        this->createArduino3Actuators(config, arduino3Driver, router.get());

        // Inject logger
        auto logger = new Com::MCULogger("arduino3", arduino3Driver.get());

        // Move logger to the logger thread
        logger->moveToThread(m_mcusLoggerThread);
        // Move MCUDriver to its own thread
        arduino3Driver->moveToThread(thread.get());

        // Wire MCUDriver connection status signals -> Machine Status Repo value update
        QObject::connect(arduino3Driver.get(), &MCUDriver::s_connected, [&]() { m_repo->setValueRaw(MCU_ARDUINO3_READY, true); });
        QObject::connect(arduino3Driver.get(), &MCUDriver::s_connectionLost, [&]() { m_repo->setValueRaw(MCU_ARDUINO3_READY, false); });
        // Wire MCUDriver -> Router
        QObject::connect(arduino3Driver.get(), &MCUDriver::s_packetReady, router.get(), &Com::PacketRouter::ps_routePacket);

        // Store in lifecycle manager
        m_subsystems[MCU_ARDUINO3_ID] = MCUSubsystemNode{
            .thread = std::move(thread),
            .driver = std::move(arduino3Driver),
            .router = std::move(router),
            .logger = logger,
        };
    }

    void HardwareManager::createArduino3Sensors(const Kub3::Config::hardware_config_t &config, Com::PacketRouter *router)
    {
        using namespace Kub3::HAL::Sensors;

        const auto getEncConvFactor = &getPositionMotorEncoderConversionFactor;

        // ===========================================
        // UPWARD PIPELINE (Hardware --> Software)
        // ===========================================

        // Create Sensors
        // --- Encoders
        auto zLeftEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, Z_LEFT_ENCODER, Z_LEFT_ENCODER_MM, &encoderValueParser, getEncConvFactor(Z_LEFT_MOTOR, config));
        auto zRightEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, Z_RIGHT_ENCODER, Z_RIGHT_ENCODER_MM, &encoderValueParser, getEncConvFactor(Z_RIGHT_MOTOR, config));
        auto zBackEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, Z_BACK_ENCODER, Z_BACK_ENCODER_MM, &encoderValueParser, getEncConvFactor(Z_BACK_MOTOR, config));
        auto maskEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, MASK_ENCODER, MASK_ENCODER_MM, &encoderValueParser, getEncConvFactor(MASK_DRAWER_MOTOR, config));
        auto waferEncoder = std::make_shared<KinematicEncoderSensor>(
            m_repo, WAFER_ENCODER, WAFER_ENCODER_MM, &encoderValueParser, getEncConvFactor(WAFER_DRAWER_MOTOR, config));
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
        auto leftForce  = std::make_shared<Sensor<uint16_t>>(m_repo, FORCE_LEFT_ADC, static_cast<uint16_t>(300), &forceSensorParser);
        auto rightForce = std::make_shared<Sensor<uint16_t>>(m_repo, FORCE_RIGHT_ADC, static_cast<uint16_t>(300), &forceSensorParser);
        auto backForce  = std::make_shared<Sensor<uint16_t>>(m_repo, FORCE_BACK_ADC, static_cast<uint16_t>(300), &forceSensorParser);

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
        this->registerSensor(router, "F1"s, std::move(leftForce));
        this->registerSensor(router, "F2"s, std::move(rightForce));
        this->registerSensor(router, "F3"s, std::move(backForce));
    }

    void HardwareManager::createArduino3Actuators(const Config::hardware_config_t &config, const std::shared_ptr<MCUDriver> &driver, Com::PacketRouter *router)
    {
        using Kinematics = Algorithms::Kinematic::KinematicGeneratorKind;

        // ===========================================
        // DOWNWARD PIPELINE (Software --> Hardware)
        // ===========================================

        /// --- Motors
        auto zLeftMotor  = createStepperMotor(config, Z_LEFT_MOTOR, '1', Kinematics::TRAPEZOIDAL, driver, Z_LEFT_ENCODER);
        auto zRightMotor = createStepperMotor(config, Z_RIGHT_MOTOR, '2', Kinematics::TRAPEZOIDAL, driver, Z_RIGHT_ENCODER);
        auto zBackMotor  = createStepperMotor(config, Z_BACK_MOTOR, '3', Kinematics::TRAPEZOIDAL, driver, Z_BACK_ENCODER);
        auto maskMotor   = createStepperMotor(config, MASK_DRAWER_MOTOR, '4', Kinematics::TRAPEZOIDAL, driver, MASK_ENCODER);
        auto waferMotor  = createStepperMotor(config, WAFER_DRAWER_MOTOR, '5', Kinematics::TRAPEZOIDAL, driver, WAFER_ENCODER);
        // --- Force sensors switches
        auto leftForceSwitch  = std::make_shared<Act::LogicSwitch>(FORCE_LEFT_SWITCH, "F11", "F10", driver);
        auto rightForceSwitch = std::make_shared<Act::LogicSwitch>(FORCE_RIGHT_SWITCH, "F21", "F20", driver);
        auto backForceSwitch  = std::make_shared<Act::LogicSwitch>(FORCE_BACK_SWITCH, "F31", "F30", driver);

        router->registerRoute("SS\x00"s, Act::StepperMotor::createFeedbackHandler(zLeftMotor));
        router->registerRoute("SS\x01"s, Act::StepperMotor::createFeedbackHandler(zRightMotor));
        router->registerRoute("SS\x02"s, Act::StepperMotor::createFeedbackHandler(zBackMotor));
        router->registerRoute("SS\x03"s, Act::StepperMotor::createFeedbackHandler(maskMotor));
        router->registerRoute("SS\x04"s, Act::StepperMotor::createFeedbackHandler(waferMotor));
        router->registerRoute("F?1", Act::LogicSwitch::createFeedbackHandler(leftForceSwitch));
        router->registerRoute("F?2", Act::LogicSwitch::createFeedbackHandler(rightForceSwitch));
        router->registerRoute("F?3", Act::LogicSwitch::createFeedbackHandler(backForceSwitch));

        m_actuatorRegistry->registerActuator(std::move(zLeftMotor));
        m_actuatorRegistry->registerActuator(std::move(zRightMotor));
        m_actuatorRegistry->registerActuator(std::move(zBackMotor));
        m_actuatorRegistry->registerActuator(std::move(maskMotor));
        m_actuatorRegistry->registerActuator(std::move(waferMotor));
        m_actuatorRegistry->registerActuator(std::move(leftForceSwitch));
        m_actuatorRegistry->registerActuator(std::move(rightForceSwitch));
        m_actuatorRegistry->registerActuator(std::move(backForceSwitch));
    }

    // --- Cameras HAL instanciation helpers

    void HardwareManager::setupCamerasSubsystem(const Config::hardware_config_t &config)
    {
        for (auto [qId, config] : config.cameras)
        {
            auto thread = std::make_unique<QThread>();
            // TODO: make a configuration-dependent camera class `C` in `std::make_shared<C>`
            auto camera = std::make_shared<Vision::HikrobotCamera>(config);

            camera->moveToThread(thread.get());
            // On connection, set default parameters to camera
            connect(
                camera.get(), &Vision::ICamera::s_cameraConnected, this,
                [cam = camera.get(), config]() {
                    QMetaObject::invokeMethod(cam, &Vision::ICamera::setCenteredZoom, Qt::QueuedConnection, 1.0);
                    QMetaObject::invokeMethod(cam, &Vision::ICamera::setExposure, Qt::QueuedConnection, config.defaultExposureUs);
                    QMetaObject::invokeMethod(cam, &Vision::ICamera::setGain, Qt::QueuedConnection, config.defaultGainDb);
                    QMetaObject::invokeMethod(cam, &Vision::ICamera::setFrameRate, Qt::QueuedConnection, config.framerate);
                });
            // Connect frame forwarding
            connect(
                camera.get(), &Vision::ICamera::s_frameReady, this,
                [&, id = QString::fromStdString(config.id)](const QImage &frame) { emit s_cameraFrameReady(id, frame); });

            m_cameras[qId] = CameraSubsystemNode{
                .thread = std::move(thread),
                .camera = std::move(camera),
            };
        }
    }

#endif // defined(KUB_MODEL_8)

    // --- Sensors HAL instanciation helpers

    void HardwareManager::registerSensor(Com::PacketRouter *router, std::string &&route, Shared<Kub3::HAL::Sensors::ISensor> sensor)
    {
        auto handler = [weakSensor = std::weak_ptr<HAL::Sensors::ISensor>(sensor)](const QByteArray &data) {
            if (auto safeSensor = weakSensor.lock())
            {
                safeSensor->processData(data);
            }
        };

        // Map the route in the router
        router->registerRoute(route, handler);
        // Store sensor to member vector
        m_sensors.push_back(std::move(sensor));
    }

    // --- Motors HAL instanciation helpers

    Shared<Act::StepperMotor> HardwareManager::createStepperMotor(
        const Config::hardware_config_t &config,
        const QString &motorId,
        uint8_t byteId,
        Algorithms::Kinematic::KinematicGeneratorKind kineGenKind,
        const Shared<MCUDriver> &driver,
        const std::string &encoderId)
    {
        auto it = config.motors.find(motorId);

        if (it == config.motors.end())
            throw std::runtime_error(std::format("Hardware configuration not found for key: '{}'", motorId.toStdString()));

        auto *hwProps = std::get_if<Config::stepper_hw_properties_t>(&it->second.hwProperties);

        if (!hwProps)
            throw std::runtime_error(std::format("'{}' configuration doesn't match expected type (stepper)", motorId.toStdString()));

        auto kinematicEngine = Algorithms::Kinematic::buildKinematicGenerator(kineGenKind);
        auto encoderGetter   = [repo = m_repo, encoderId]() { return HAL::MS::readInt32(repo, encoderId); };

        m_registeredMotorIds.push_back(motorId.toStdString());
        return std::make_shared<Act::StepperMotor>(
            it->second.id, byteId, driver, *hwProps,
            std::move(encoderGetter), encoderId,
            std::move(kinematicEngine));
    }

    Shared<Act::DirectCurrentMotor> HardwareManager::createDCMotor(
        const Config::hardware_config_t &config,
        const QString &motorId,
        uint8_t byteId,
        Algorithms::Kinematic::KinematicGeneratorKind kineGenKind,
        const Shared<MCUDriver> &driver)
    {
        auto it = config.motors.find(motorId);

        if (it == config.motors.end())
            throw std::runtime_error(std::format("Hardware configuration not found for key: '{}'", motorId.toStdString()));

        auto *hwProps = std::get_if<Config::dc_motor_hw_properties_t>(&it->second.hwProperties);

        if (!hwProps)
            throw std::runtime_error(std::format("'{}' configuration doesn't match expected type (direct current)", motorId.toStdString()));

        auto kinematicEngine = Algorithms::Kinematic::buildKinematicGenerator(kineGenKind);

        m_registeredMotorIds.push_back(motorId.toStdString());
        return std::make_shared<Act::DirectCurrentMotor>(it->second.id, byteId, driver, *hwProps, std::move(kinematicEngine));
    }

} // namespace Kub3::HAL

// Key extractors

static std::string_view arduino1KeyExtractor(const Kub3::HAL::Com::packet_t &packet)
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
    default:
    {
        if (('1' <= payload[0] && payload[0] <= '6') || payload[0] == '8')
            return std::string_view(payload.data(), 1);
        break;
    }
    }
    return std::string_view(); // 404
}

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
    return !d.isEmpty() && (d[0] != 0x0 && d[0] != '0');
}

static bool valveStatusParser(const QByteArray &d)
{
    return !d.isEmpty() && d[0] != '0';
}

static bool pressureSensorParser(const QByteArray &d)
{
    return !d.isEmpty() && d[0] != '0';
}

static double temperatureParser(const QByteArray &d)
{
    if (d.size() < 2) // Not enough data to read
        return INT16_MIN;
    // Big-endian reconstruction
    int16_t vADC = (static_cast<int16_t>(static_cast<uint8_t>(d[0])) << 8) |
                   (static_cast<int16_t>(static_cast<uint8_t>(d[1])));
    double vPin       = (static_cast<double>(vADC) * 3.3) / 4095.0;
    double vOutSensor = vPin * 1.545454545454;
    double tempC      = (vOutSensor - 1.375) / 0.0225;
    return tempC;
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

    // Big-endian
    // Reconstruct using unsigned 32-bit integers to safely shift bits
    const uint32_t rawUnsigned =
        (static_cast<uint32_t>(static_cast<uint8_t>(d[0])) << 24) |
        (static_cast<uint32_t>(static_cast<uint8_t>(d[1])) << 16) |
        (static_cast<uint32_t>(static_cast<uint8_t>(d[2])) << 8) |
        (static_cast<uint32_t>(static_cast<uint8_t>(d[3])));

    // Safely cast the fully assembled bit-pattern to a signed integer as bit shifting
    // on signed integers is undefined behaviour
    return static_cast<int32_t>(rawUnsigned);
}

static uint16_t fansVoltageParser(const QByteArray &d)
{
    if (d.size() < 2) // Not enough data to read
        return 0;
    // Big-endian 2 bytes reconstruction
    return (static_cast<uint16_t>(static_cast<uint8_t>(d[0])) << 8) |
           (static_cast<uint16_t>(static_cast<uint8_t>(d[1])));
}

static uint32_t ledLineVoltageParser(const QByteArray &d)
{
    if (d.size() < 4) // Not enough data to read
        return 0;
    // Big-endian reconstruction
    return
        // Voltage 1: ??? (see communication protocol)
        (static_cast<uint32_t>(static_cast<uint8_t>(d[0])) << 24) |
        (static_cast<uint32_t>(static_cast<uint8_t>(d[1])) << 16) |
        // Voltage 2: resistor's limits
        (static_cast<uint32_t>(static_cast<uint8_t>(d[2])) << 8) |
        (static_cast<uint32_t>(static_cast<uint8_t>(d[3])));
}

static bool physicalButtonParser(const QByteArray &d)
{
    return true; // @note: Receiving message only when button was pressed
}

// Helpers

static double getPositionMotorEncoderConversionFactor(const QString &motorId, const Kub3::Config::hardware_config_t &config)
{
    if (auto it = config.motors.find(motorId); it != config.motors.end())
    {
        if (auto *p = std::get_if<Kub3::Config::stepper_hw_properties_t>(&it->second.hwProperties))
        {
            return p->screwPitchMm / static_cast<double>(p->encoderTopsPerRev);
        }
    }
    return 1.0;
};
