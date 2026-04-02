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
        void registerActuator(Shared<IActuator> actuator)
        {
            if (!actuator)
                throw std::invalid_argument("Cannot register null actuator");
            m_actuators.emplace(actuator->getId(), std::move(actuator));
        }

        // Getter for actuators by id
        template <typename T>
        [[nodiscard]] Shared<T> get(std::string_view id) const
        {
            static_assert(std::is_base_of_v<IActuator, T>, "T must inherit from IActuator");

            if (auto it = m_actuators.find(id); it != m_actuators.end())
            {
                auto ptr = std::dynamic_pointer_cast<T>(it->second);
                if (!ptr)
                {
                    // TODO: Optional instead of throw ?
                    throw std::runtime_error("Actuator type mismatch for ID: " + std::string(id));
                }
                return ptr;
            }
            throw std::out_of_range("Actuator not found: " + std::string(id));
        }

    private:
        using ActuatorMap = std::unordered_map<std::string, Shared<IActuator>, Utils::StringViewHash, std::equal_to<>>;

        ActuatorMap m_actuators;
    };

} // namespace Kub3::HAL::Act
