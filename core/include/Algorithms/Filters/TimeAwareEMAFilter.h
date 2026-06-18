#pragma once

#include <cmath>

namespace Kub3::Algorithms::Filter
{
    /**
     * @brief A Time-Aware Exponential Moving Average (First-Order Low-Pass Filter).
     * Highly resilient to loop jitter because it dynamically recalculates alpha using dt.
     */
    class LowPassFilter
    {
    public:
        // cutoffFrequencyHz dictates how aggressively to filter.
        // Lower = smoother but more lag. Higher = faster but more noise.
        explicit LowPassFilter(double cutoffFrequencyHz = 10.0);
        void reset(double initialValue = 0.0);
        double update(double rawValue, double dt);

        [[nodiscard]] double get() const
        {
            return m_value;
        }

    private:
        double m_tau;
        double m_value     = 0.0;
        bool m_initialized = false;
    };

} // namespace Kub3::Math
