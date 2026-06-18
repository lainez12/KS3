#pragma once

#include <QObject>
#include <QStringList>
#include <QTimer>
#include <memory>
#include <string>

#include <HAL/Actuators/Motors/IMotor.h>
#include <Services/MotorTestService.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{

    class MotorTestController : public QObject
    {
        Q_OBJECT

    public:
        explicit MotorTestController(Shared<HAL::Act::ActuatorRegistry> actuatorRegistry,
                                     std::vector<std::string> knownMotorsIds,
                                     QObject *parent = nullptr);
        ~MotorTestController() override;

        MotorTestService *getMotorTestService(void)
        {
            return m_motorTestService.get();
        }

    public slots:
        // System lifecycle
        void start();
        void stop();

        // Commands from UI
        void ps_selectMotor(const QString &motorId);
        void ps_startJog(int direction, Kub3::Config::kinematic_profile_t profile);
        void ps_stopJog();
        void ps_moveToAbsolute(double positionMm, Kub3::Config::kinematic_profile_t profile);
        void ps_emergencyStopAll();
        void ps_onMachineStatusUpdate(const std::string &key);

    signals:
        // Pushed to ViewModel
        void s_telemetryUpdated(double positionMm, double speedMmS, double accelMmS2);
        void s_availableMotorsDiscovered(const QStringList &motors);
        void s_motorSelectionChanged(bool isValid);

    private:
        Unique<MotorTestService> m_motorTestService;

        Shared<HAL::Act::ActuatorRegistry> m_actuatorRegistry;
        std::vector<std::string> m_knownMotorsIds;
    };

} // namespace Kub3::Tools::Tester
