#include <QDebug>
#include <cmath>

#include <Common/Clock.h>
#include <Services/Contact/tasks/AdmittanceControlTask.h>

namespace Kub3::Services
{

    using Dir = HAL::Act::MotorDirection;

    AdmittanceControlTask::AdmittanceControlTask(std::array<Shared<HAL::Act::IPositionMotor>, 3> motors,
                                                 std::function<force_readings_t()> forceGetter,
                                                 std::function<void(const std::string &)> abortCb,
                                                 Algorithms::Control::admittance_config_t config,
                                                 Mode mode,
                                                 Config::kinematic_profile_t baseProfile) :
        m_motors(std::move(motors)),
        m_forceGetter(std::move(forceGetter)),
        m_abortCb(std::move(abortCb)),
        m_config(std::move(config)),
        m_mode(mode),
        m_baseProfile(std::move(baseProfile))
    {
    }

    void AdmittanceControlTask::start(void)
    {
        m_isFirstTick   = true;
        m_lastTickNsecs = 0;
        m_dtTimer.start();
    }

    bool AdmittanceControlTask::tick(void)
    {
        using namespace Algorithms::Control;

        double dt_sec = LOGIC_TIMER_PERIOD_MS / 1000.0; // Default to logic timer period on first tick

        // Calculate precise dt (handle first tick edge-case and OS jitter)
        if (!m_isFirstTick)
        {
            const int64_t currentNsecs = m_dtTimer.nsecsElapsed();

            dt_sec          = static_cast<double>(currentNsecs - m_lastTickNsecs) / 1e9; // convert to seconds
            m_lastTickNsecs = currentNsecs;
        }
        m_isFirstTick = false;

        force_readings_t forces    = m_forceGetter(); // Sample Sensors
        admittance_input_t math_in = {{forces.left, forces.right, forces.back}, dt_sec};
        admittance_output_t math_out; // Algorithmic Evaluation

        if (m_mode == Mode::Autoleveling)
            math_out = AdmittanceController::computeAutoleveling(m_config, math_in);
        else
            math_out = AdmittanceController::computeBasicContact(m_config, math_in);

        // Handle Hardware State based on Math Truth
        if (math_out.limit_exceeded_abort)
        {
            for (auto &motor : m_motors)
            {
                if (motor)
                    motor->emergencyStop();
            }
            // TODO: Go down instead of abort ?
            m_abortCb("CRITICAL: Process force limit exceeded during admittance control.");
            return true;
        }

        // Finish condition met, stop motors and exit
        if (math_out.is_converged)
        {
            // Start convergence confirmation timer
            if (!m_convergenceTimer.isValid())
            {
                m_convergenceTimer.start();
            }

            // Wait for 1 second to confirm planeity reached
            if (m_convergenceTimer.elapsed() > 1000) // in milliseconds
            {
                for (auto &motor : m_motors)
                {
                    if (motor)
                        motor->emergencyStop();
                }
                return true; // Exit
            }

            return false;
        }
        m_convergenceTimer.invalidate();

        // Command Continuous Velocities to HAL layer
        auto applyVelocity = [this](Shared<HAL::Act::IPositionMotor> &motor, double velocity) {
            if (!motor)
                return;

            // If the math output 0 (e.g. inside digital deadband), stop the motor
            if (velocity == 0.0)
            {
                motor->emergencyStop();
                return;
            }

            // Direction dictates Up (+) or Down (-)
            auto dir = (velocity > 0.0) ? Dir::Positive : Dir::Negative;
            // Generate mutated profile containing the target velocity
            Config::kinematic_profile_t dyn_profile = m_baseProfile;

            dyn_profile.targetVelocityMmS = std::abs(velocity);
            motor->moveDirection(dir, dyn_profile);
        };

        qDebug().noquote() << "Admittance velocities_mm_s:" << math_out.velocities_mm_s[0]
                           << math_out.velocities_mm_s[1]
                           << math_out.velocities_mm_s[2];

        applyVelocity(m_motors[0], math_out.velocities_mm_s[0]);
        applyVelocity(m_motors[1], math_out.velocities_mm_s[1]);
        applyVelocity(m_motors[2], math_out.velocities_mm_s[2]);

        return false; // Not converged, tick again next cycle
    }

} // namespace Kub3::Services
