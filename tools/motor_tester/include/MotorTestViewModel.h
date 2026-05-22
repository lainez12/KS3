#pragma once

#include <QList>
#include <QObject>
#include <QPair>
#include <QStringList>

#include <Config/kinematics.h>

namespace Kub3::Tools::MotorTester
{

    class MotorTestViewModel : public QObject
    {
        Q_OBJECT
        // Q_PROPERTY(double position READ position NOTIFY s_telemetryChanged)
        // Q_PROPERTY(double speed READ speed NOTIFY s_telemetryChanged)
        // Q_PROPERTY(double acceleration READ acceleration NOTIFY s_telemetryChanged)
        // Q_PROPERTY(bool hasValidMotor READ hasValidMotor NOTIFY s_hasValidMotorChanged)
        // Q_PROPERTY(bool isStepper READ isStepper NOTIFY s_isStepperChanged)

    public:
        explicit MotorTestViewModel(QObject *parent = nullptr);

        [[nodiscard]] double position() const noexcept
        {
            return m_position;
        }
        [[nodiscard]] double speed() const noexcept
        {
            return m_speed;
        }
        [[nodiscard]] double acceleration() const noexcept
        {
            return m_acceleration;
        }
        [[nodiscard]] bool hasValidMotor() const noexcept
        {
            return m_hasValidMotor;
        }
        [[nodiscard]] bool isStepper() const noexcept
        {
            return m_isStepper;
        }

        void setAvailableMotors(const QList<QPair<QString, bool>> &motors);

    public slots:
        // UI Action Inputs (Notice they construct the profile on the fly)
        void uiSelectMotor(int index);
        void uiJogHold(int direction, double velocity, double accel, int stepFraction);
        void uiJogRelease();
        void uiMoveToAbsolute(double targetMm, double velocity, double accel, int stepFraction);
        void uiEmergencyStop();

        // Inbound Service Telemetry
        void onTelemetryUpdated(double pos, double spd, double acc);
        void onMotorSelectionChanged(bool isValid);

    signals:
        // UI Property Notifiers
        void s_telemetryChanged();
        void s_hasValidMotorChanged();
        void s_isStepperChanged();
        void s_availableMotorsLoaded(const QStringList &motorIds);

        // Outbound Commands to Logic Thread Controller
        void cmdSelectMotor(const QString &motorId);
        void cmdJog(int direction, Kub3::Config::kinematic_profile_t profile);
        void cmdStopJog();
        void cmdMoveToAbsolute(double positionMm, Kub3::Config::kinematic_profile_t profile);
        void cmdEmergencyStopAll();

    private:
        Kub3::Config::kinematic_profile_t buildProfile(double vel, double acc, int stepFrac) const;

        double m_position     = 0.0;
        double m_speed        = 0.0;
        double m_acceleration = 0.0;

        bool m_hasValidMotor = false;
        bool m_isStepper     = false;

        QList<QPair<QString, bool>> m_motorCatalog; // <MotorID, isStepper>
    };

} // namespace Kub3::Tools::MotorTester
