#include "MotorTestController.h"

namespace Kub3::Tools::MotorTester
{

    MotorTestController::MotorTestController(Unique<MotorTestService> service, QObject *parent) :
        QObject(parent),
        m_service(std::move(service))
    {
    }

    MotorTestController::~MotorTestController()
    {
        if (m_service)
            m_service->stop();
    }

    void MotorTestController::start()
    {
        // 1. Publish available motors on startup
        QStringList motorList;
        for (const auto &id : m_service->getAvailableMotors())
        {
            motorList << QString::fromStdString(id);
        }
        emit s_availableMotorsDiscovered(motorList);

        // 2. Start 50Hz Polling Timer
        m_tickTimer = new QTimer(this);
        m_tickTimer->setInterval(20);
        connect(m_tickTimer, &QTimer::timeout, this, &MotorTestController::onTick);
        m_tickTimer->start();
    }

    void MotorTestController::stop()
    {
        if (m_tickTimer)
            m_tickTimer->stop();
        m_service->stop();
    }

    void MotorTestController::onTick()
    {
        m_service->tick();
        if (m_service->hasValidMotor())
        {
            auto tel = m_service->getTelemetry();
            emit s_telemetryUpdated(tel.positionMm, tel.speedMmS, tel.accelMmS2);
        }
    }

    void MotorTestController::ps_selectMotor(const QString &motorId)
    {
        bool valid = m_service->selectMotor(motorId.toStdString());
        emit s_motorSelectionChanged(valid);
    }

    void MotorTestController::ps_startJog(int direction, Kub3::Config::kinematic_profile_t profile)
    {
        auto dir = static_cast<HAL::Act::MotorDirection>(direction);
        m_service->startJog(dir, profile);
    }

    void MotorTestController::ps_stopJog()
    {
        m_service->stopJog();
    }

    void MotorTestController::ps_moveToAbsolute(double positionMm, Kub3::Config::kinematic_profile_t profile)
    {
        m_service->moveToAbsolute(positionMm, profile);
    }

    void MotorTestController::ps_emergencyStopAll()
    {
        m_service->emergencyStopAll();
        emit s_motorSelectionChanged(false);
    }

} // namespace Kub3::Tools::MotorTester