#include <HAL/Sensors/Sensor.h>

namespace Kub3::HAL::Sensors
{

    /**
     * @class KinematicEncoderSensor
     * @brief Specialized encoder sensor implementing a 1-route-to-multiple-values ingestion pattern.
     *
     * This class encapsulates the dual-state representation of physical motor encoders.
     * A single hardware packet received over a communications route represents a single physical
     * event, but produces two distinct semantic states required by the machine status repository:
     *   1. The raw integer tick count (used for low-level motion tracking and actuator feedback).
     *   2. The converted physical position in floating-point units (millimeters).
     *
     * Why this class exists:
     * - **1 Route -> Several Values Pattern:** Keeps the underlying PacketRouter mapping strictly
     *   1-to-1 (one route key per callback) to preserve zero-overhead packet dispatching while
     *   simultaneously updating multiple repository fields atomically.
     * - **Inline Kinematic Conversion:** Encapsulates the kinematic properties (screw pitch and
     *   encoder resolution) directly at the data ingestion boundary, converting raw hardware data
     *   into physical units in a single pass without requiring external polling or translation services.
     */
    class KinematicEncoderSensor final : public Sensor<int32_t>
    {
    public:
        KinematicEncoderSensor(Shared<MS::IMachineStatusRepo> repo,
                               const std::string &rawKey,
                               const std::string &physicalKey,
                               Mapper mapper,
                               double conversionFactor) :
            Sensor<int32_t>(repo, rawKey, 0, std::move(mapper)),
            m_physicalKey(physicalKey),
            m_conversionFactor(conversionFactor)
        {
            m_repo->setValueRaw(m_physicalKey, 0.0);
        }

        void processData(const QByteArray &data) override
        {
            // 1. Base class updates the raw int32_t in the repo
            Sensor<int32_t>::processData(data);

            // 2. Re-parse and push the value converted to millimeters using
            int32_t rawTicks   = m_mapper ? m_mapper(data) : 0;
            double physicalPos = static_cast<double>(rawTicks) * m_conversionFactor;

            m_repo->setValueRaw(m_physicalKey, physicalPos);
        }

    private:
        std::string m_physicalKey;
        double m_conversionFactor;
    };

}
