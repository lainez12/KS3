#pragma once

#include <cmath>

#include "IKinematicGenerator.h"

namespace Kub3::Algorithms::Kinematic
{

    class TrapezoidalGenerator final : public IKinematicGenerator
    {
    public:
        TrapezoidalGenerator()           = default;
        ~TrapezoidalGenerator() override = default;

        // Delete copy/move ctor semantics to enforce strict single-ownership
        TrapezoidalGenerator(const TrapezoidalGenerator &)            = delete;
        TrapezoidalGenerator &operator=(const TrapezoidalGenerator &) = delete;
        TrapezoidalGenerator(TrapezoidalGenerator &&)                 = delete;
        TrapezoidalGenerator &operator=(TrapezoidalGenerator &&)      = delete;

        // Start a move to a specific absolute position
        void startPositionMove(double currentPosition, double targetPosition, double maxVelocity, double acceleration) override;
        // Start a continuous velocity move (directionSign: 1.0 for forward, -1.0 for reverse)
        void startVelocityMove(double currentPosition, double directionSign, double maxVelocity, double acceleration) override;

        // Update parameters for a move to a specific absolute position
        void updatePositionMove(double targetPosition, double maxVelocity, double acceleration) override;
        // Update a continuous velocity move (directionSign: 1.0 for forward, -1.0 for reverse)
        void updateVelocityMove(double directionSign, double maxVelocity, double acceleration) override;

        // Gracefully ramps the velocity down to 0 at the current acceleration rate
        void commandSmoothStop(void) override;

        kinematic_state_t calculateNext(double dt) override;

        [[nodiscard]] kinematic_state_t getCurrentState() const noexcept override
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
