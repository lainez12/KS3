#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <iostream>

#include "HAL/Actuators/Motors/TrapezoidalGenerator.h"

using namespace Kub3::HAL::Act;
using Catch::Matchers::WithinAbs;

// Helper to log kinematic state
static void logKinematicState(const kinematic_state_t &state)
{
    std::cout << "[STATE] Pos.: " << state.position << "mm | Vel.: " << state.velocity << "mm/s" << std::endl;
}

TEST_CASE("TrapezoidalGenerator Physics Integration", "[kinematics][math]")
{
    TrapezoidalGenerator generator;

    SECTION("Accurate Physics Integration (Trapezoidal Rule)")
    {
        const double initialPos = 0.0, targetPos = 100.0, velocity = 10.0, acceleration = 10.0;

        // 100mm move, 10mm/s max vel, 10mm/s^2 accel.
        // 1s to reach 10mm/s. Distance in accel = 0.5 * 10 * 1^2 = 5mm.
        generator.startPositionMove(initialPos, targetPos, velocity, acceleration);

        // Simulate exactly 1 second of acceleration in 50Hz (20ms) ticks
        for (int i = 0; i < 50; ++i)
            generator.calculateNext(0.02);

        kinematic_state_t state = generator.calculateNext(0.0); // Peek state

        REQUIRE_THAT(state.velocity, WithinAbs(10.0, 0.001));
        REQUIRE_THAT(state.position, WithinAbs(5.0, 0.001)); // Proves integration is correct
    }

    SECTION("Reaches Target and Snaps to Stop")
    {
        const double initialPos = 0.0, targetPos = 50.0, velocity = 20.0, acceleration = 70.0;

        generator.startPositionMove(initialPos, targetPos, velocity, acceleration);

        kinematic_state_t state = {.isFinished = false};
        int ticks               = 0;

        // std::cerr << "finished: " << state.isFinished << " ticks:" << ticks << std::endl;
        while (!state.isFinished && ticks++ < 1000)
        {
            state = generator.calculateNext(0.02);
            // std::cerr << "Position update: " << state.position << "mm | Speed: " << state.velocity << "mm/s" << std::endl;
        }

        REQUIRE(state.isFinished == true);
        REQUIRE_THAT(state.velocity, WithinAbs(0.0, 0.001));
        REQUIRE_THAT(state.position, WithinAbs(50.0, 0.001));
    }

    SECTION("Smooth Stop Interrupt")
    {
        kinematic_state_t state = {.isFinished = false};
        const double initialPos = 0.0, targetPos = 1.0, velocity = 50.0, acceleration = 10.0;

        generator.startVelocityMove(initialPos, targetPos, velocity, acceleration); // Infinite move
        for (int i = 0; i < 100; ++i)
            generator.calculateNext(0.02); // Cruise
        generator.commandSmoothStop();

        while (!state.isFinished)
            state = generator.calculateNext(0.02);

        REQUIRE(state.isFinished == true);
        REQUIRE_THAT(state.velocity, WithinAbs(0.0, 0.001));
    }

    //////////////////////////////////
    // On the fly updates testing
    //////////////////////////////////

    SECTION("On-the-fly Velocity Blending (Deceleration while cruising)")
    {
        kinematic_state_t state = {.isFinished = false};
        const double initialPos = 0.0, targetPos = 1.0, velocity = 100.0, acceleration = 100.0;

        // Start cruising at 100mm/s
        generator.startVelocityMove(initialPos, targetPos, velocity, acceleration);

        // Tick for 1.5 seconds. Velocity should reach and settle 100.0mm/s
        for (int i = 0; i < 75; ++i)
            generator.calculateNext(0.02);

        state = generator.getCurrentState();
        REQUIRE_THAT(state.velocity, WithinAbs(100.0, 0.001));

        // INJECT NEW PROFILE: Slow down to 20mm/s
        generator.updateVelocityMove(1.0, 20.0, 50.0);

        // Tick for 2 seconds. It should decelerate at 50mm/s^2 and settle at 20.0mm/s
        for (int i = 0; i < 100; ++i)
            generator.calculateNext(0.02);

        state = generator.getCurrentState();
        REQUIRE(state.isFinished == false);
        REQUIRE_THAT(state.velocity, WithinAbs(20.0, 0.001)); // Proves smooth deceleration to new target
    }

    SECTION("On-the-fly Target Extension (Position Mode)")
    {
        kinematic_state_t state = {.isFinished = false};
        double initialPos = 0.0, targetPos = 50.0, velocity = 20.0, acceleration = 100.0;

        // Start a move to 50mm
        generator.startPositionMove(initialPos, targetPos, velocity, acceleration);

        // Tick exactly halfway (Pos ~25mm)
        for (int i = 0; i < 60; ++i)
            generator.calculateNext(0.02);

        state = generator.getCurrentState();
        REQUIRE(state.position > 20.0);
        REQUIRE(state.position < 30.0);
        REQUIRE(state.velocity > 0.0); // We are moving

        // INJECT NEW PROFILE: Extend target to 100mm, keep cruising
        targetPos = 100.0;
        generator.updatePositionMove(targetPos, velocity, acceleration);

        // Tick until finished
        int ticks = 0;
        while (!state.isFinished && ticks++ < 1000)
        {
            state = generator.calculateNext(0.02);
        }

        // Proves it seamlessly continued to the new target
        REQUIRE(state.isFinished == true);
        REQUIRE_THAT(state.position, WithinAbs(100.0, 0.1));
    }

    SECTION("On-the-fly Target Extension (Position Mode + Direction Switch)")
    {
        kinematic_state_t state = {.isFinished = false};
        double initialPos = 0.0, targetPos = 50.0, velocity = 20.0, acceleration = 100.0;

        // Start a move to 50mm
        generator.startPositionMove(initialPos, targetPos, velocity, acceleration);

        // Tick exactly halfway (Pos ~25mm)
        for (int i = 0; i < 60; ++i)
        {
            state = generator.calculateNext(0.02);
        }

        state = generator.getCurrentState();
        REQUIRE(state.position > 20.0);
        REQUIRE(state.position < 30.0);
        REQUIRE(state.velocity > 0.0); // We are moving

        // INJECT NEW PROFILE: Set target to -10mm
        targetPos = -10.0;
        generator.updatePositionMove(targetPos, velocity, acceleration);

        // Tick until finished
        int ticks = 0;
        while (!state.isFinished && ticks++ < 1000)
        {
            state = generator.calculateNext(0.02);
        }

        // Proves it seamlessly continued to the new target
        REQUIRE(state.isFinished == true);
        REQUIRE_THAT(state.position, WithinAbs(-10.0, 0.1));
    }

    SECTION("Mode Switching (Velocity -> Position on the fly)")
    {
        kinematic_state_t state = {.isFinished = false};

        generator.startVelocityMove(0.0, 1.0, 50.0, 50.0);

        for (int i = 0; i < 50; ++i)
        {
            state = generator.calculateNext(0.02); // Let it cruise
        }

        REQUIRE_THAT(generator.getCurrentState().velocity, WithinAbs(50.0, 0.001));

        generator.updatePositionMove(100.0, 50.0, 50.0);

        int ticks = 0;
        while (!state.isFinished && ticks++ < 1000)
        {
            state = generator.calculateNext(0.02);
        }

        // Proves the math engine successfully switched from infinite cruise to a targeted stop
        REQUIRE(state.isFinished == true);
        REQUIRE_THAT(state.position, WithinAbs(100.0, 0.1));
        REQUIRE_THAT(state.velocity, WithinAbs(0.0, 0.001));
    }
}
