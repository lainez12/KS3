#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <iostream>

#include <Algorithms/Admittance/AdmittanceController.h>

using namespace Kub3::Algorithms::Control;
using Catch::Matchers::WithinAbs;

static admittance_config_t getBaseConfig()
{
    return admittance_config_t{
        .target_force_gf        = 500.0,
        .force_tolerance_gf     = 5.0,
        .max_process_force_gf   = 1000.0,
        .k_mean_max             = 0.005, // Fast: 0.005 mm/s per gram
        .k_mean_min             = 0.001, // Slow: 0.001 mm/s per gram
        .k_tilt_max             = 0.010,
        .k_tilt_min             = 0.002,
        .max_step_mm_per_tick   = 0.025, // 25 microns max per tick
        .max_profile_speed_mm_s = 1.0,   // 1 mm/s absolute max hardware limit
        .min_profile_speed_mm_s = 0.001  // Digital deadband
    };
}

TEST_CASE("Admittance Controller Safety Limits", "[admittance][safety]")
{
    auto config = getBaseConfig();

    SECTION("Aborts when Process Limit is exceeded")
    {
        admittance_input_t input = {{1200.0, 400.0, 400.0}, 0.02}; // Left is 1200, exceeding max 1000

        admittance_output_t out_basic = AdmittanceController::computeBasicContact(config, input);
        admittance_output_t out_auto  = AdmittanceController::computeAutoleveling(config, input);

        REQUIRE(out_basic.limit_exceeded_abort == true);
        REQUIRE(out_auto.limit_exceeded_abort == true);
    }

    SECTION("Limits velocity when DT spikes (Stall Protection)")
    {
        // Average force is 0, so e_max = 500. Velocity request = 500 * K_max = 2.5 mm/s.
        admittance_input_t input = {{0.0, 0.0, 0.0}, 0.5}; // Emulate 500ms thread stall

        admittance_output_t out = AdmittanceController::computeBasicContact(config, input);

        // Safe max velocity for 0.5s = max_step (0.025) / 0.5s = 0.05 mm/s
        REQUIRE_THAT(out.velocities_mm_s[0], WithinAbs(0.05, 0.001));
        REQUIRE_THAT(out.velocities_mm_s[1], WithinAbs(0.05, 0.001));
        REQUIRE_THAT(out.velocities_mm_s[2], WithinAbs(0.05, 0.001));
    }

    SECTION("Deadband enforces 0.0 velocity for micro-noises")
    {
        admittance_input_t input = {{498.0, 498.0, 498.0}, 0.02};
        // Force error is 2 gF.
        // Ratio = 498/500 ~ 1.0. K_mean ~ 0.001.
        // Expected vel = 0.001 * 2 = 0.002 mm/s.

        // Let's artificially increase the deadband to 0.010 mm/s
        config.min_profile_speed_mm_s = 0.010;

        // Drop the tolerance to 1GF so it does not converge, but deadband catches it.
        config.force_tolerance_gf = 1.0;

        admittance_output_t out = AdmittanceController::computeBasicContact(config, input);

        REQUIRE(out.is_converged == false);
        REQUIRE_THAT(out.velocities_mm_s[0], WithinAbs(0.0, 0.0001));
    }
}

TEST_CASE("Basic Contact Mode (SISO)", "[admittance][basic_contact]")
{
    auto config = getBaseConfig();

    SECTION("Applies identical synchronous velocities for all motors")
    {
        // Highly imbalanced forces, but Basic Contact MUST ignore tilt and move synchronously
        admittance_input_t input = {{400.0, 0.0, 0.0}, 0.02};

        admittance_output_t out = AdmittanceController::computeBasicContact(config, input);

        REQUIRE(out.is_converged == false);
        REQUIRE_THAT(out.velocities_mm_s[0], WithinAbs(out.velocities_mm_s[1], 0.0001));
        REQUIRE_THAT(out.velocities_mm_s[1], WithinAbs(out.velocities_mm_s[2], 0.0001));
        REQUIRE(out.velocities_mm_s[0] > 0.0); // Moving UP
    }

    SECTION("Converges on the Max Sensor")
    {
        // Target is 500. Left is 496. Right/Back are 0. (Max = 496). Error = 4 <= 5 (tolerance).
        admittance_input_t input = {{496.0, 0.0, 0.0}, 0.02};

        admittance_output_t out = AdmittanceController::computeBasicContact(config, input);
        REQUIRE(out.is_converged == true);
    }

    SECTION("Handles Overshoot Gracefully (Moves Downwards)")
    {
        // Target is 500. Max force is 600. Target missed or starting too high.
        admittance_input_t input = {{600.0, 600.0, 600.0}, 0.02};

        admittance_output_t out = AdmittanceController::computeBasicContact(config, input);

        // Error is -100. It must NOT converge, and velocities MUST be negative.
        REQUIRE(out.is_converged == false);
        REQUIRE(out.velocities_mm_s[0] < 0.0);
    }
}

TEST_CASE("Autoleveling Mode (WEC/MIMO)", "[admittance][autoleveling]")
{
    auto config = getBaseConfig();

    SECTION("Adjusts mean perfectly when no tilt exists")
    {
        admittance_input_t input = {{400.0, 400.0, 400.0}, 0.02};

        admittance_output_t out = AdmittanceController::computeAutoleveling(config, input);

        REQUIRE(out.is_converged == false);
        // All velocities should be identical and positive
        REQUIRE_THAT(out.velocities_mm_s[0], WithinAbs(out.velocities_mm_s[1], 0.0001));
        REQUIRE_THAT(out.velocities_mm_s[1], WithinAbs(out.velocities_mm_s[2], 0.0001));
        REQUIRE(out.velocities_mm_s[0] > 0.0);
    }

    SECTION("Twists the stage perfectly when mean is exactly at target")
    {
        // Target = 500. Forces: [600, 400, 500].
        // Mean = 500. e_mean = 0.
        // e_L = 500 - 600 = -100.
        // e_R = 500 - 400 = +100.
        // e_B = 500 - 500 = 0.
        admittance_input_t input = {{600.0, 400.0, 500.0}, 0.02};

        admittance_output_t out = AdmittanceController::computeAutoleveling(config, input);

        REQUIRE(out.is_converged == false);
        // Left should move down (negative speed)
        REQUIRE(out.velocities_mm_s[0] < 0.0);
        // Right should move up (positive speed)
        REQUIRE(out.velocities_mm_s[1] > 0.0);
        // Back is perfectly at the target average, its velocity should be near zero
        REQUIRE_THAT(out.velocities_mm_s[2], WithinAbs(0.0, 0.0001));
    }

    SECTION("Combined Mean + Tilt resolution")
    {
        // Target = 500.
        // Mean is 400. Stage generally needs to go up.
        // But Left is 600.
        admittance_input_t input = {{600.0, 300.0, 300.0}, 0.02};

        admittance_output_t out = AdmittanceController::computeAutoleveling(config, input);

        REQUIRE(out.is_converged == false);
        // Because Left is overloaded (e_L = -200), the suspension (tilt) subtraction
        // should overcome the elevator (mean) addition, forcing Left down.
        REQUIRE(out.velocities_mm_s[0] < 0.0);
        // Right and Back are way below average (e = +100), they should rush upwards.
        REQUIRE(out.velocities_mm_s[1] > 0.0);
        REQUIRE(out.velocities_mm_s[2] > 0.0);
    }

    SECTION("Converges only when both Mean AND Tilt are strictly within tolerance")
    {
        // Target = 500, Tol = 5.
        // Mean = 498. e_mean = +2. (Mean is converged)
        // However, Left is 490 and Right is 506.
        // e_L = 498 - 490 = +8. (Tilt is NOT converged)
        admittance_input_t input = {{490.0, 506.0, 498.0}, 0.02};

        admittance_output_t out = AdmittanceController::computeAutoleveling(config, input);

        REQUIRE(out.is_converged == false);

        // Now if they are all within 5 of the mean (e.g. 496, 500, 498)...
        admittance_input_t input_converged = {{496.0, 500.0, 498.0}, 0.02};
        admittance_output_t out_converged  = AdmittanceController::computeAutoleveling(config, input_converged);

        REQUIRE(out_converged.is_converged == true);
    }
}
