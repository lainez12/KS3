#include <algorithm>

#include "HAL/Actuators/Motors/TrapezoidalGenerator.h"

namespace Kub3::HAL::Act
{

    void TrapezoidalGenerator::startPositionMove(double currentPosition, double targetPosition, double maxVelocity, double acceleration)
    {
        m_mode             = Mode::Position;
        m_state.position   = currentPosition;
        m_state.velocity   = 0.0;
        m_state.isFinished = false;

        m_targetPosition = targetPosition;
        m_maxVelocity    = std::abs(maxVelocity);
        m_acceleration   = std::abs(acceleration);
        m_stopRequested  = false;
    }

    void TrapezoidalGenerator::startVelocityMove(double currentPosition, double directionSign, double maxVelocity, double acceleration)
    {
        m_mode             = Mode::Velocity;
        m_state.position   = currentPosition;
        m_state.velocity   = 0.0;
        m_state.isFinished = false;

        m_directionSign = (directionSign >= 0.0) ? 1.0 : -1.0;
        m_maxVelocity   = std::abs(maxVelocity);
        m_acceleration  = std::abs(acceleration);
        m_stopRequested = false;
    }

    void TrapezoidalGenerator::updatePositionMove(double targetPosition, double maxVelocity, double acceleration)
    {
        m_mode             = Mode::Position;
        m_state.isFinished = false;

        m_targetPosition = targetPosition;
        m_maxVelocity    = std::abs(maxVelocity);
        m_acceleration   = std::abs(acceleration);
        m_stopRequested  = false;
    }

    void TrapezoidalGenerator::updateVelocityMove(double directionSign, double maxVelocity, double acceleration)
    {
        m_mode             = Mode::Velocity;
        m_state.isFinished = false;

        m_directionSign = (directionSign >= 0.0) ? 1.0 : -1.0;
        m_maxVelocity   = std::abs(maxVelocity);
        m_acceleration  = std::abs(acceleration);
        m_stopRequested = false;
    }

    void TrapezoidalGenerator::commandSmoothStop(void)
    {
        m_stopRequested = true;
    }

    // `dt` is expected to be in seconds
    kinematic_state_t TrapezoidalGenerator::calculateNext(double dt)
    {
        if (m_state.isFinished)
            return m_state;

        double targetV = 0.0;

        // Determine our IDEAL target velocity for this exact millisecond
        if (m_mode == Mode::Position)
        {
            double distanceToGo = m_targetPosition - m_state.position;
            double dir          = (distanceToGo >= 0.0) ? 1.0 : -1.0;

            // Physics: d = v^2 / 2a (Distance required to stop exactly at 0 velocity)
            double stoppingDistance = (m_state.velocity * m_state.velocity) / (2.0 * m_acceleration);
            // DISCRETE LOOKAHEAD:
            // Add the distance we would travel in this tick if we didn't brake.
            // This guarantees we never overshoot the deceleration curve.
            double lookaheadStoppingDistance = stoppingDistance + (std::abs(m_state.velocity) * dt);

            if (m_stopRequested || std::abs(distanceToGo) <= lookaheadStoppingDistance)
                targetV = 0.0; // Decelerate!
            else
                targetV = m_maxVelocity * dir; // Cruise / Accelerate
        }
        else // Mode::Velocity
        {
            if (m_stopRequested)
                targetV = 0.0; // Decelerate
            else
                targetV = m_maxVelocity * m_directionSign; // Cruise forever
        }

        const double prevVelocity = m_state.velocity;

        // Apply Acceleration limits (Step the velocity towards targetV)
        if (m_state.velocity < targetV)
        {
            m_state.velocity += m_acceleration * dt;
            if (m_state.velocity > targetV)
                m_state.velocity = targetV; // Clamp
        }
        else if (m_state.velocity > targetV)
        {
            m_state.velocity -= m_acceleration * dt;
            if (m_state.velocity < targetV)
                m_state.velocity = targetV; // Clamp
        }

        // Integrate Velocity to update Position (d = v * t)
        m_state.position += (prevVelocity + m_state.velocity) * 0.5 * dt;

        // Check for Completion (Velocity reached 0)
        if (std::abs(m_state.velocity) < 0.001)
        {
            m_state.velocity = 0.0; // Prevent floating-point micro-jitter

            if (m_mode == Mode::Position)
            {
                if (m_stopRequested || std::abs(m_targetPosition - m_state.position) < 0.01)
                {
                    m_state.position   = m_targetPosition; // Snap to exact target position
                    m_state.isFinished = true;
                }
            }
            else if (m_mode == Mode::Velocity && m_stopRequested)
            {
                m_state.isFinished = true;
            }
        }

        return m_state;
    }

} // namespace Kub3::HAL::Act
