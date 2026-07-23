#pragma once

#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include <MFSM/states.operational.h>

namespace Kub3::MFSM
{
    struct StateBooting {
        uint32_t ticksElapsed = 0;
        std::unordered_map<std::string, int8_t> retryCounts;
    };

    struct StateWaitingInitialization {};

    struct StateInitializing {};

    struct StateOperational {
        SystemPosture posture;
        OperationalState subState = StateIdle{};
    };

    struct StateError {
        SystemPosture posture; // Last known posture
        ErrorSeverity severity = ErrorSeverity::Critical;
        std::string message;
        ErrorAction allowedActions;
    };

    struct StateEmergencyStop {
        SystemPosture posture; // Last known posture
        std::string reason;
    };

    struct StatePreparePowerOff {};

    struct StatePowerOff {};

    enum class SystemStateKind
    {
        Booting,
        WaitingInitialization,
        Initializing,
        Operational,
        Error,
        EmergencyStop,
        PreparePowerOff,
        PowerOff
    };

    using SystemState = std::variant<
        StateBooting,
        StateWaitingInitialization,
        StateInitializing,
        StateOperational,
        StateError,
        StateEmergencyStop,
        StatePreparePowerOff,
        StatePowerOff>;

    inline SystemStateKind kindOf(const SystemState &state)
    {
        return std::visit(
            [](const auto &s) -> SystemStateKind {
                using T = std::decay_t<decltype(s)>;

                if constexpr (std::is_same_v<T, StateBooting>)
                    return SystemStateKind::Booting;
                else if constexpr (std::is_same_v<T, StateWaitingInitialization>)
                    return SystemStateKind::WaitingInitialization;
                else if constexpr (std::is_same_v<T, StateInitializing>)
                    return SystemStateKind::Initializing;
                else if constexpr (std::is_same_v<T, StateOperational>)
                    return SystemStateKind::Operational;
                else if constexpr (std::is_same_v<T, StateError>)
                    return SystemStateKind::Error;
                else if constexpr (std::is_same_v<T, StateEmergencyStop>)
                    return SystemStateKind::EmergencyStop;
                else if constexpr (std::is_same_v<T, StatePreparePowerOff>)
                    return SystemStateKind::PreparePowerOff;
                else
                    return SystemStateKind::PowerOff;
            },
            state);
    }
}
