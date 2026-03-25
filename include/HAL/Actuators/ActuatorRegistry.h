#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#include "IActuator.h"
#include "utils.h"

namespace Kub3::HAL::Act
{

    class ActuatorRegistry
    {
    public:
        ActuatorRegistry()  = default;
        ~ActuatorRegistry() = default;

        // Delete copy/move semantics to enforce a single authoritative instance per HardwareManager
        ActuatorRegistry(const ActuatorRegistry &)            = delete;
        ActuatorRegistry &operator=(const ActuatorRegistry &) = delete;

        // Registers an actuator
        void registerActuator(Shared<IActuator> actuator);

        // Getter for actuators by id
        template <typename T>
        [[nodiscard]] Shared<T> get(std::string_view id) const;

    private:
        using ActuatorMap = std::unordered_map<std::string, Shared<IActuator>, Utils::StringViewHash, std::equal_to<>>;

        ActuatorMap m_actuators;
    };

} // namespace Kub3::HAL::Act
