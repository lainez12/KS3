#include <QDebug>
#include <QString>
#include <mutex>

#include "HAL/MachineStatus/MachineStatusRepo.h"

namespace Kub3::HAL::MS
{

    MachineStatusRepo::MachineStatusRepo(QObject *parent) : IMachineStatusRepo(parent)
    {
    }

    void MachineStatusRepo::setSensorRaw(const std::string &key, const SensorValue &value)
    {
        bool valueChanged = false;

        {
            std::unique_lock lock(m_mutex); // scoped lock
            auto it = m_sensors.find(key);  // Find value if any

            if (it != m_sensors.end())
            {
                if (it->second != value)
                {
                    it->second   = value;
                    valueChanged = true;
                }
            }
            else
            {
                // Dynamic population: inserting a new sensor key
                m_sensors.emplace(key, value);
                valueChanged = true;
            }
        }

        // Emit the signal OUTSIDE the mutex lock.
        // If a connected Qt Slot reacts to this signal by calling getSensor(),
        // it would deadlock if we still held the exclusive write lock.
        if (valueChanged)
        {
            emit sensorValueChanged(QString::fromStdString(key));
        }
    }

    Optional<SensorValue> MachineStatusRepo::getSensorRaw(const std::string &key) const
    {
        // Acquire a shared lock (Read Lock,  multiple readers allowed)
        std::shared_lock lock(m_mutex);
        auto it = m_sensors.find(key);

        if (it != m_sensors.end())
        {
            return it->second; // Returns a copy of the variant (safe to use outside lock)
        }

        return std::nullopt; // Key does not exist
    }

    std::vector<std::string> MachineStatusRepo::getRegisteredKeys() const
    {
        std::shared_lock lock(m_mutex);
        std::vector<std::string> keys;

        keys.reserve(m_sensors.size());

        for (const auto &[key, _] : m_sensors)
            keys.push_back(key);

        return keys;
    }

} // namespace Kub3::HAL::MS
