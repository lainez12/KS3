/*
 * ADMITTANCE:
 * - Inverse of impedance
 * - Force (input) -> Velocity (output)
 */

#pragma once

#include <array>

namespace Kub3::Algorithms::Control
{

    // Injectable configuration for the admittance control loop
    struct admittance_config_t {
        double target_force_gf;
        double force_tolerance_gf;
        double max_process_force_gf; // Absolute limit to trigger abort

        // Gain Scheduling (Compliance: mm/s per GF)
        double k_mean_max; // Used when force is 0 (fast/soft approach)
        double k_mean_min; // Used when force is at target (slow/stiff approach)
        double k_tilt_max; // Only used in Autoleveling (WEC)
        double k_tilt_min; // Only used in Autoleveling (WEC)

        // Safety Limits
        double max_step_mm_per_tick;   // E.g., 0.025 mm (25 microns)
        double max_profile_speed_mm_s; // Hardware upper limit
        double deadband_velocity_mm_s; // Hardware digital deadband
    };

    struct admittance_input_t {
        std::array<double, 3> forces_gf; // Strictly [Left, Right, Back]
        double dt_seconds;               // Time elapsed since last tick to cap distance
    };

    struct admittance_output_t {
        std::array<double, 3> velocities_mm_s; // Strictly [Left, Right, Back]
        bool is_converged;
        bool limit_exceeded_abort;
    };

    class AdmittanceController
    {
    public:
        /**
         * @brief Computes Wedge Error Compensation (Multiple Inputs Multiple Outputs Admittance).
         * @details Twists the stage to equalize forces across all 3 sensors while
         *          targeting the requested mean force.
         */
        [[nodiscard]] static admittance_output_t computeAutoleveling(
            const admittance_config_t &config,
            const admittance_input_t &input) noexcept;

        /**
         * @brief Computes Synchronous Plane-Preserving Contact (Single Input Single Ouput Admittance).
         * @details Moves all 3 motors at the exact same velocity to preserve the
         *          initial mechanical plane. Evaluates convergence strictly on
         *          the maximum sensor reading reaching the target.
         */
        [[nodiscard]] static admittance_output_t computeBasicContact(
            const admittance_config_t &config,
            const admittance_input_t &input) noexcept;
    };

} // namespace Kub3::Algorithms::Control
