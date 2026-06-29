#include <ViewModels/MotorTestViewModel.h>

namespace Kub3::Tools::Tester
{

    MotorTestViewModel::MotorTestViewModel(QObject *parent) : QObject(parent) {}

    void MotorTestViewModel::setAvailableMotors(const QList<QPair<QString, bool>> &motors)
    {
        m_motorCatalog = motors;
        QStringList ids;
        for (const auto &m : motors)
            ids << m.first;
        emit s_availableMotorsLoaded(ids);
    }

    Kub3::Config::kinematic_profile_t MotorTestViewModel::buildProfile(double vel, double acc, int stepFrac) const
    {
        Kub3::Config::kinematic_profile_t profile;
        profile.id                = "custom_test_profile";
        profile.targetVelocityMmS = vel;
        profile.accelerationMmS2  = acc;

        if (m_isStepper)
        {
            Config::stepper_kinematics_params_t stepperParams;
            stepperParams.stepFraction = static_cast<uint8_t>(stepFrac);
            profile.params             = stepperParams;
        }
        else
        {
            profile.params = std::monostate{};
        }
        return profile;
    }

    void MotorTestViewModel::uiSelectMotor(int index)
    {
        if (index < 0 || index >= m_motorCatalog.size())
            return;

        const auto &[motorId, isStepper] = m_motorCatalog[index];

        if (m_isStepper != isStepper)
        {
            m_isStepper = isStepper;
            emit s_isStepperChanged();
        }

        emit cmdSelectMotor(motorId);
    }

    void MotorTestViewModel::uiJogHold(int direction, double velocity, double accel, int stepFraction)
    {
        emit cmdJog(direction, buildProfile(velocity, accel, stepFraction));
    }

    void MotorTestViewModel::uiJogRelease()
    {
        emit cmdStopJog();
    }

    void MotorTestViewModel::uiMoveToAbsolute(double targetMm, double velocity, double accel, int stepFraction)
    {
        emit cmdMoveToAbsolute(targetMm, buildProfile(velocity, accel, stepFraction));
    }

    void MotorTestViewModel::uiEmergencyStop()
    {
        emit cmdEmergencyStopAll();
    }

    void MotorTestViewModel::onTelemetryUpdated(double pos, double spd, double acc)
    {
        m_position     = pos;
        m_speed        = spd;
        m_acceleration = acc;
        emit s_telemetryChanged();
    }

    void MotorTestViewModel::onMotorSelectionChanged(bool isValid)
    {
        if (m_hasValidMotor != isValid)
        {
            m_hasValidMotor = isValid;
            emit s_hasValidMotorChanged();
        }
    }

} // namespace Kub3::Tools::Tester
