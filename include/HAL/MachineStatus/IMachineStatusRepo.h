#pragma once

#include <QDebug>
#include <QObject>
#include <string>
#include <variant>
#include <vector>

#include "utils.h"

namespace Kub3::HAL::MS
{

    // Define all allowed types for our sensors.
    using SensorValue = std::variant<bool, int32_t, uint32_t>;

    class IMachineStatusRepo : public QObject
    {
        Q_OBJECT
    public:
        explicit IMachineStatusRepo(QObject *parent = nullptr) : QObject(parent)
        {
        }
        virtual ~IMachineStatusRepo() = default;

        // Core virtual interface
        virtual void setSensorRaw(const std::string &key, const SensorValue &value)            = 0;
        [[nodiscard]] virtual Optional<SensorValue> getSensorRaw(const std::string &key) const = 0;
        [[nodiscard]] virtual std::vector<std::string> getRegisteredKeys() const               = 0;

        template <typename T>
        void setSensor(const std::string &key, T value) // TODO: check type safety
        {
            setSensorRaw(key, SensorValue(value));
        }

        template <typename T>
        [[nodiscard]] Optional<T> getSensor(const std::string &key) const
        {
            auto optVal = getSensorRaw(key);

            if (!optVal)
            {
                return std::nullopt; // Key doesn't exist
            }
            // Check if the variant holds the requested type
            if (const T *val = std::get_if<T>(&(*optVal)))
            {
                return *val;
            }
            return std::nullopt; // Type mismatch
        }

    signals:
        void sensorValueChanged(const QString &key);
    };

} // namespace Kub3::HAL::MS
