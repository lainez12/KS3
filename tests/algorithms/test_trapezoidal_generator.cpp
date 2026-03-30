#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <iostream>

#include "HAL/Actuators/Motors/TrapezoidalGenerator.h"

using namespace Kub3::HAL::Act;
using Catch::Matchers::WithinAbs;

TEST_CASE("TrapezoidalGenerator Physics Integration", "[kinematics][math]")
{
    TrapezoidalGenerator generator;

    SECTION("Accurate Physics Integration (Trapezoidal Rule)")
    {
        // 100mm move, 10mm/s max vel, 10mm/s^2 accel.
        // 1s to reach 10mm/s. Distance in accel = 0.5 * 10 * 1^2 = 5mm.
        generator.startPositionMove(0.0, 100.0, 10.0, 10.0);

        // Simulate exactly 1 second of acceleration in 50Hz (20ms) ticks
        for (int i = 0; i < 50; ++i)
            generator.calculateNext(0.02);

        kinematic_state_t state = generator.calculateNext(0.0); // Peek state

        REQUIRE_THAT(state.velocity, WithinAbs(10.0, 0.001));
        REQUIRE_THAT(state.position, WithinAbs(5.0, 0.001)); // Proves integration is correct
    }

    SECTION("Reaches Target and Snaps to Stop")
    {
        generator.startPositionMove(0.0, 50.0, 20.0, 70.0);

        kinematic_state_t state = {.isFinished = false};
        int ticks               = 0;

        std::cerr << "finished: " << state.isFinished << " ticks:" << ticks << std::endl;
        while (!state.isFinished && ticks++ < 1000)
        {
            state = generator.calculateNext(0.02);
            std::cerr << "Position update: " << state.position << "mm | Speed: " << state.velocity << "mm/s" << std::endl;
        }

        REQUIRE(state.isFinished == true);
        REQUIRE_THAT(state.velocity, WithinAbs(0.0, 0.001));
        REQUIRE_THAT(state.position, WithinAbs(50.0, 0.001));
    }

    SECTION("Smooth Stop Interrupt")
    {
        kinematic_state_t state = {.isFinished = false};

        generator.startVelocityMove(0.0, 1.0, 50.0, 10.0); // Infinite move
        for (int i = 0; i < 100; ++i)
            generator.calculateNext(0.02); // Cruise
        generator.commandSmoothStop();

        while (!state.isFinished)
            state = generator.calculateNext(0.02);

        REQUIRE(state.isFinished == true);
        REQUIRE_THAT(state.velocity, WithinAbs(0.0, 0.001));
    }
}
