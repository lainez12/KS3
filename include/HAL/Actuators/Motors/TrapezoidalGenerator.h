#pragma once

#include <cmath>

namespace Kub3::HAL::Act
{

    typedef struct kinematic_state_s
    {
        double position = 0.0;
        double velocity = 0.0;
        bool isFinished = true;
    } kinematic_state_t;

    class TrapezoidalGenerator
    {
    public:
        enum class Mode
        {
            Position,
            Velocity
        };

        TrapezoidalGenerator() = default;

        // Start a move to a specific absolute position
        void startPositionMove(double currentPosition, double targetPosition, double maxVelocity, double acceleration);
        // Start a continuous velocity move (directionSign: 1.0 for forward, -1.0 for reverse)
        void startVelocityMove(double currentPosition, double directionSign, double maxVelocity, double acceleration);
        // Gracefully ramps the velocity down to 0 at the current acceleration rate
        void commandSmoothStop();

        kinematic_state_t calculateNext(double dt);

        [[nodiscard]] kinematic_state_t getCurrentState() const noexcept
        {
            return m_state;
        }

    private:
        kinematic_state_t m_state;
        Mode m_mode = Mode::Position;

        double m_targetPosition = 0.0;
        double m_maxVelocity    = 0.0;
        double m_acceleration   = 0.0;
        double m_directionSign  = 1.0;

        bool m_stopRequested = false;
    };

} // namespace Kub3::HAL::Act
