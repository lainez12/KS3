#pragma once

namespace Kub3::Algorithms::Kinematic
{

    typedef struct kinematic_state_s {
        double position = 0.0;
        double velocity = 0.0;
        bool isFinished = true;
    } kinematic_state_t;

    class IKinematicGenerator
    {
    public:
        enum class Mode
        {
            Position,
            Velocity
        };

        virtual ~IKinematicGenerator() = default;

        // Start a move to a specific absolute position
        virtual void startPositionMove(double currentPosition, double targetPosition, double maxVelocity, double acceleration) = 0;
        // Start a continuous velocity move (directionSign: 1.0 for forward, -1.0 for reverse)
        virtual void startVelocityMove(double currentPosition, double directionSign, double maxVelocity, double acceleration) = 0;

        // Update parameters for a move to a specific absolute position
        virtual void updatePositionMove(double targetPosition, double maxVelocity, double acceleration) = 0;
        // Update a continuous velocity move (directionSign: 1.0 for forward, -1.0 for reverse)
        virtual void updateVelocityMove(double directionSign, double maxVelocity, double acceleration) = 0;

        // Gracefully ramps the velocity down to 0 at the current acceleration rate
        virtual void commandSmoothStop(void) = 0;

        virtual kinematic_state_t calculateNext(double dt) = 0;

        [[nodiscard]] virtual kinematic_state_t getCurrentState() const noexcept = 0;
    };

}
