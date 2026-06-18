#include <QThread>

#include <Controllers/MotorTestController.h>

namespace Kub3::Tools::Tester
{

    MotorTestController::MotorTestController(Shared<HAL::Act::ActuatorRegistry> actuatorRegistry,
                                             std::vector<std::string> knownMotorsIds,
                                             QObject *parent) :
        QObject(parent),
        m_actuatorRegistry(std::move(actuatorRegistry)),
        m_knownMotorsIds(std::move(std::vector(knownMotorsIds)))
    {
    }

    MotorTestController::~MotorTestController()
    {
        if (m_motorTestService)
            m_motorTestService->stop();
    }

    void MotorTestController::start()
    {
        m_motorTestService = std::make_unique<MotorTestService>(m_actuatorRegistry, m_knownMotorsIds); // Create here to be inside thread

        // Publish available motors on startup
        QStringList motorList;

        for (const auto &id : m_motorTestService->getAvailableMotors())
        {
            motorList << QString::fromStdString(id);
        }

        emit s_availableMotorsDiscovered(motorList);
    }

    void MotorTestController::stop()
    {
        m_motorTestService->stop();
    }

    void MotorTestController::ps_onMachineStatusUpdate(const std::string &sensorId)
    {
        if (m_motorTestService->hasValidMotor() && m_motorTestService->getCurrentEncoderId() == sensorId)
        {
            m_motorTestService->tick();

            const auto tel = m_motorTestService->getTelemetry();
            emit s_telemetryUpdated(tel.positionMm, tel.speedMmS, tel.accelMmS2);
        }
    }

    void MotorTestController::ps_selectMotor(const QString &motorId)
    {
        bool valid = m_motorTestService->selectMotor(motorId.toStdString());
        emit s_motorSelectionChanged(valid);
    }

    void MotorTestController::ps_startJog(int direction, Kub3::Config::kinematic_profile_t profile)
    {
        auto dir = static_cast<HAL::Act::MotorDirection>(direction);
        m_motorTestService->startJog(dir, profile);
    }

    void MotorTestController::ps_stopJog()
    {
        m_motorTestService->stopJog();
    }

    void MotorTestController::ps_moveToAbsolute(double positionMm, Kub3::Config::kinematic_profile_t profile)
    {
        m_motorTestService->moveToAbsolute(positionMm, profile);
    }

    void MotorTestController::ps_emergencyStopAll()
    {
        m_motorTestService->emergencyStopAll();
    }

} // namespace Kub3::Tools::Tester