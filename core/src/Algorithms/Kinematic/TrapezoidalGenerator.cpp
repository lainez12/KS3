#include <QDebug>
#include <algorithm>

#include <Algorithms/Kinematic/TrapezoidalGenerator.h>

namespace Kub3::Algorithms::Kinematic
{

    void TrapezoidalGenerator::startPositionMove(double currentPosition, double targetPosition, double maxVelocity, double acceleration, double precisionMm)
    {
        m_mode             = Mode::Position;
        m_state.position   = currentPosition;
        m_state.velocity   = 0.0;
        m_state.isFinished = false;

        m_precisionMm    = precisionMm;
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

    void TrapezoidalGenerator::updatePositionMove(double targetPosition, double maxVelocity, double acceleration, double precisionMm)
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

    kinematic_state_t TrapezoidalGenerator::computeNext(double dt)
    {
        if (m_state.isFinished)
            return m_state;

        if (m_mode == Mode::Position)
        {
            this->positionModeComputeNext(dt); // Updates `m_state`
        }
        else // Mode::Velocity
        {
            this->velocityModeComputeNext(dt); // Updates `m_state`
        }

        return m_state;
    }

    // `dt` is expected to be in seconds
    kinematic_state_t TrapezoidalGenerator::computeNext(double dt, double realPosition)
    {
        const kinematic_state_t prevState = m_state;
        kinematic_state_t state           = this->computeNext(dt);

        if (state.isFinished && m_mode == Mode::Position)
        {
            const double absDistToTarget = std::abs(realPosition - m_targetPosition);

            // Position mode finished but real position is not within precision range
            if (absDistToTarget > m_precisionMm)
            {
                m_state          = prevState;
                m_state.position = realPosition;
                state            = this->computeNext(dt); // Re-compute state using real position
            }
        }
        return state;
    }

    void TrapezoidalGenerator::positionModeComputeNext(double dt)
    {
        const double distanceToGo    = m_targetPosition - m_state.position;
        const double absDistanceToGo = std::abs(distanceToGo);
        const double dir             = (distanceToGo >= 0.0) ? 1.0 : -1.0;

        // Check if we are already within the precision window
        if (absDistanceToGo < m_precisionMm)
        {
            m_state.position   = m_targetPosition;
            m_state.velocity   = 0.0;
            m_state.isFinished = true;
            return;
        }

        const double absVel = std::abs(m_state.velocity);

        // Handle starting from "no-movements"
        if (absVel < 1e-8)
        {
            const double maxNudgeVel   = absDistanceToGo / dt; // Speed to perform to reach destination within `dt`
            const double nominalAccVel = m_acceleration * dt;

            m_state.velocity = dir * std::min({maxNudgeVel, nominalAccVel, m_maxVelocity}); // Accelerate from "no-movements"
            m_state.position += m_state.velocity * 0.5 * dt;
            return;
        }

        // Physics: d = v^2 / 2a (Distance required to stop exactly at 0 velocity)
        const double stoppingDistance = (absVel * absVel) / (2.0 * m_acceleration);
        // DISCRETE LOOKAHEAD:
        // - Add the distance we would travel in this tick if we didn't brake.
        // - This guarantees to not overshoot the deceleration curve.
        const double lookaheadStoppingDistance = stoppingDistance + (absVel * dt);

#if defined(BUILD_DEBUG)
        qDebug() << "Distance to go (mm):" << distanceToGo;
        qDebug() << "Look ahead stopping distance (mm):" << lookaheadStoppingDistance;
#endif

        if (m_stopRequested || absDistanceToGo <= lookaheadStoppingDistance)
        {
            // Calculate the exact deceleration rate needed to stop on target
            const double a_req = (absVel * absVel) / (2.0 * absDistanceToGo);
            // Calculate the physical time remaining until we hit zero speed
            const double t_stop = (2.0 * absDistanceToGo) / absVel;

            if (t_stop <= dt)
            {
                // The target is reached during this timestep. Land exactly on target.
                m_state.position   = m_targetPosition;
                m_state.velocity   = 0.0;
                m_state.isFinished = true;
            }
            else
            {
                // Decelerate using the adapted deceleration rate
                const double prevVelocity = m_state.velocity;

                if (m_state.velocity > 0.0)
                {
                    m_state.velocity -= a_req * dt;
                    if (m_state.velocity < 0.0)
                        m_state.velocity = 0.0; // Cap
                }
                else
                {
                    m_state.velocity += a_req * dt;
                    if (m_state.velocity > 0.0)
                        m_state.velocity = 0.0; // Cap
                }

                // Integrate Velocity to update Position (d = v * t)
                m_state.position += (prevVelocity + m_state.velocity) * 0.5 * dt;
            }
        }
        else
        {
            // Cruise / Acceleration Zone
            const double targetV      = m_maxVelocity * dir;
            const double prevVelocity = m_state.velocity;

            if (m_state.velocity < targetV)
            {
                m_state.velocity += m_acceleration * dt;
                if (m_state.velocity > targetV)
                    m_state.velocity = targetV;
            }
            else if (m_state.velocity > targetV)
            {
                m_state.velocity -= m_acceleration * dt;
                if (m_state.velocity < targetV)
                    m_state.velocity = targetV;
            }

            // Integrate Velocity to update Position (d = v * t)
            m_state.position += (prevVelocity + m_state.velocity) * 0.5 * dt;
        }
    }

    void TrapezoidalGenerator::velocityModeComputeNext(double dt)
    {
        double targetV            = m_stopRequested ? 0.0 : (m_maxVelocity * m_directionSign);
        const double prevVelocity = m_state.velocity;

        if (m_state.velocity < targetV) // Need to accelerate
        {
            m_state.velocity += m_acceleration * dt;
            if (m_state.velocity > targetV)
                m_state.velocity = targetV; // Cap
        }
        else if (m_state.velocity > targetV) // Need to decelerate
        {
            m_state.velocity -= m_acceleration * dt;
            if (m_state.velocity < targetV)
                m_state.velocity = targetV; // Cap
        }

        // Integrate Velocity to update Position (d = v * t)
        m_state.position += (prevVelocity + m_state.velocity) * 0.5 * dt;

        if (m_stopRequested && std::abs(m_state.velocity) < 1e-6)
        {
            m_state.velocity   = 0.0;
            m_state.isFinished = true;
        }
    }

} // namespace Kub3::Algorithms::Kinematic
