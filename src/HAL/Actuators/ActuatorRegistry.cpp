#include "HAL/Actuators/ActuatorRegistry.h"

namespace Kub3::HAL::Act
{

    void ActuatorRegistry::registerActuator(Shared<IActuator> actuator)
    {
        if (!actuator)
            throw std::invalid_argument("Cannot register null actuator");
        m_actuators.emplace(actuator->getId(), std::move(actuator));
    }

    template <typename T>
    [[nodiscard]] Shared<T> ActuatorRegistry::get(std::string_view id) const
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

} // namespace Kub3::HAL::Act
