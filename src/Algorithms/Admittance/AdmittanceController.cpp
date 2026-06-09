#include <QtLogging>
#include <algorithm>
#include <cmath>

#if defined(BUILD_DEBUG)
#include <QDebug>
#include <QString>
#endif

#include <Algorithms/Admittance/AdmittanceController.h>

namespace Kub3::Algorithms::Control
{

    // ===================================================
    // HELPER FUNCTIONS (Anonymous namespace)
    // ===================================================
    namespace
    {
        // Returns the max value in the array
        inline double getMax(const std::array<double, 3> &forces) noexcept
        {
            return std::max({forces[0], forces[1], forces[2]});
        }

        // Calculates the stiffness ratio [0.0; 1.0] to throttle speed under pressure
        inline double getStiffnessRatio(double max_f, double target_f) noexcept
        {
            return std::clamp(max_f / target_f, 0.0, 1.0);
        }

        // Safely bounds the requested velocity against hardware and dt limits
        inline double clampVelocity(double v, double dt, const admittance_config_t &config) noexcept
        {
            if (std::abs(v) < config.min_profile_speed_mm_s)
                return 0.0;

            // Protection against division-by-zero or negative time travel:
            // - If dt is extremely low (e.g. < 1ms), cap it at 1ms.
            // - This prevents safe_max_v from spiking to infinity, though std::min catches it later anyway.
            const double safe_dt = std::max(dt, 0.001);

            // Calculate the max velocity allowed so we don't travel more than max_step_mm_per_tick
            const double safe_max_v_for_dt = config.max_step_mm_per_tick / safe_dt;

            // Ultimate limit is the lowest of the hardware limit and the dt-safety limit
            const double dynamic_v_max = std::min(config.max_profile_speed_mm_s, safe_max_v_for_dt);

            return std::clamp(v, -dynamic_v_max, dynamic_v_max);
        }
    }

    // =====================================================================================
    // AUTOLEVELING (Wedge Error Compensation - WEC) - Multiple-Input Multiple-Output (MIMO)
    // =====================================================================================

    admittance_output_t AdmittanceController::computeAutoleveling(
        const admittance_config_t &config,
        const admittance_input_t &input) noexcept
    {
        admittance_output_t out = {{0.0, 0.0, 0.0}, false, false};
        const double max_f      = getMax(input.forces_gf); // Current max applied force

        // Process Limit
        if (max_f > config.max_process_force_gf)
        {
            out.limit_exceeded_abort = true;
            return out; // Abort
        }

        // Compute mean and errors
        const double mean_f     = (input.forces_gf[0] + input.forces_gf[1] + input.forces_gf[2]) / 3.0; // Mean applied force
        const double e_mean     = config.target_force_gf - mean_f;                                      // Mean error
        const double e_LRelMean = mean_f - input.forces_gf[0];                                          // Left error relative to mean
        const double e_RRelMean = mean_f - input.forces_gf[1];                                          // Right error relative to mean
        const double e_BRelMean = mean_f - input.forces_gf[2];                                          // Back error relative to mean
        const double e_L        = config.target_force_gf - input.forces_gf[0];                          // Left error relative to target
        const double e_R        = config.target_force_gf - input.forces_gf[1];                          // Right error relative to target
        const double e_B        = config.target_force_gf - input.forces_gf[2];                          // Back error relative to target

        // Detect convergence
        const bool mean_converged = std::abs(e_mean) <= config.force_tolerance_gf;
        const bool tilt_converged = (std::abs(e_L) <= config.force_tolerance_gf &&
                                     std::abs(e_R) <= config.force_tolerance_gf &&
                                     std::abs(e_B) <= config.force_tolerance_gf);

#if defined(BUILD_DEBUG)
        qDebug().noquote()
            << QString("[AdmittanceController] Target: %1gF (tolerance: %7gF);"
                       "\n\tMean force: %2gF;"
                       "\n\tMean Err: %3gF (Error to mean=[L: %4, R: %5, B: %6]);"
                       "\n\tSingle sensor Err: [L: %8, R: %9, B: %10]")
                   .arg(config.target_force_gf)
                   .arg(mean_f)
                   .arg(e_mean)
                   .arg(e_LRelMean)
                   .arg(e_RRelMean)
                   .arg(e_BRelMean)
                   .arg(config.force_tolerance_gf)
                   .arg(e_L)
                   .arg(e_R)
                   .arg(e_B);
#endif

        if (mean_converged && tilt_converged)
        {
#if defined(BUILD_DEBUG)
            qDebug() << "[AdmittanceController] convergence reached";
#endif

            out.is_converged = true;
            return out;
        }

        // Global Stiffness Scaling (Protecting the highest pressure point)
        const double ratio         = getStiffnessRatio(max_f, config.target_force_gf);
        const double k_mean_active = config.k_mean_max - (ratio * (config.k_mean_max - config.k_mean_min));
        const double k_tilt_active = config.k_tilt_max - (ratio * (config.k_tilt_max - config.k_tilt_min));

        // Compute MIMO Velocities
        // If e_mean is negative (overshot), this yields negative velocities (move down)
        out.velocities_mm_s[0] = clampVelocity((k_mean_active * e_mean) + (k_tilt_active * e_LRelMean), input.dt_seconds, config);
        out.velocities_mm_s[1] = clampVelocity((k_mean_active * e_mean) + (k_tilt_active * e_RRelMean), input.dt_seconds, config);
        out.velocities_mm_s[2] = clampVelocity((k_mean_active * e_mean) + (k_tilt_active * e_BRelMean), input.dt_seconds, config);

#if defined(BUILD_DEBUG)
        qDebug().noquote() << QString("\t--- Stiffness ratio: %3; k_mean_active: %1 ; k_tilt_active: %2").arg(k_mean_active).arg(k_tilt_active).arg(ratio);
#endif

        return out;
    }

    // =========================================================================
    // BASIC CONTACT - Single-Input Single-Output (Synchronous)
    // =========================================================================

    admittance_output_t AdmittanceController::computeBasicContact(
        const admittance_config_t &config,
        const admittance_input_t &input) noexcept
    {
        admittance_output_t out = {{0.0, 0.0, 0.0}, false, false};
        const double max_f      = getMax(input.forces_gf);

        // Process Limit
        if (max_f > config.max_process_force_gf)
        {
            out.limit_exceeded_abort = true;
            return out; // Abort
        }

        // Synchronous Error Math (Based strictly on highest point)
        const double e_max = config.target_force_gf - max_f;

        // Convergence (If highest point reaches target)
        if (std::abs(e_max) <= config.force_tolerance_gf)
        {
            out.is_converged = true;
            return out;
        }

        // Global Stiffness Scaling
        const double ratio         = getStiffnessRatio(max_f, config.target_force_gf);
        const double k_mean_active = config.k_mean_max - (ratio * (config.k_mean_max - config.k_mean_min));

        // Compute SISO Velocity
        const double sync_velocity = clampVelocity(k_mean_active * e_max, input.dt_seconds, config);

        // Output strictly identical velocities to preserve initial plane
        out.velocities_mm_s[0] = sync_velocity;
        out.velocities_mm_s[1] = sync_velocity;
        out.velocities_mm_s[2] = sync_velocity;

        return out;
    }

} // namespace Kub3::Algorithms::Control
