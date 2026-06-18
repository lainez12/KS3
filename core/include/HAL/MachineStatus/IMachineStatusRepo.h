#pragma once

#include <QDebug>
#include <QObject>
#include <string>
#include <variant>
#include <vector>

#include <utils.h>

namespace Kub3::HAL::MS
{

    // Define all allowed types for our sensors and other machine values...
    using MachineValue = std::variant<bool, int32_t, uint16_t, uint32_t>;

    class IMachineStatusRepo : public QObject
    {
        Q_OBJECT
    public:
        explicit IMachineStatusRepo(QObject *parent = nullptr) : QObject(parent)
        {
        }
        virtual ~IMachineStatusRepo() = default;

        // Core virtual interface
        virtual void setValueRaw(const std::string &key, const MachineValue &value)            = 0;
        [[nodiscard]] virtual Optional<MachineValue> getValueRaw(const std::string &key) const = 0;
        [[nodiscard]] virtual std::vector<std::string> getRegisteredKeys() const               = 0;

        template <typename T>
        void setValue(const std::string &key, T value)
        {
            setValueRaw(key, MachineValue(value));
        }

        template <typename T>
        [[nodiscard]] Optional<T> getValue(const std::string &key) const
        {
            auto optVal = getValueRaw(key);

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
        void s_machineValueChanged(const std::string &key);
    };

} // namespace Kub3::HAL::MS
