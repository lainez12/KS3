#pragma once

#include <string_view>

namespace Kub3::HAL::Act
{

    class IActuator
    {
    public:
        virtual ~IActuator() = default;

        // Every actuator must be identifiable
        [[nodiscard]] virtual std::string_view getId() const noexcept = 0;

        // Universal safety constraints
        virtual void emergencyStop()    = 0;
        virtual void enable(bool state) = 0;
    };

} // namespace KUB3::HAL
