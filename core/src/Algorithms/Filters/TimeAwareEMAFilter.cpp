#include <Algorithms/Filters/TimeAwareEMAFilter.h>

namespace Kub3::Algorithms::Filter
{
    LowPassFilter::LowPassFilter(double cutoffFrequencyHz)
    {
        if (cutoffFrequencyHz <= 0.0)
        {
            cutoffFrequencyHz = 1.0;
        }
        // Calculate the time constant (Tau)
        m_tau = 1.0 / (2.0 * M_PI * cutoffFrequencyHz);
    }

    void LowPassFilter::reset(double initialValue)
    {
        m_value       = initialValue;
        m_initialized = true;
    }

    double LowPassFilter::update(double rawValue, double dt)
    {
        if (!m_initialized || dt <= 0.0)
        {
            m_value       = rawValue;
            m_initialized = true;
            return m_value;
        }

        // Dynamically calculate alpha based on exact dt
        double alpha = dt / (m_tau + dt);

        // Apply EMA formula
        m_value = m_value + alpha * (rawValue - m_value);
        return m_value;
    }

} // namespace Kub3::Algorithms::Filter
