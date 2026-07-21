#pragma once

#include <string>
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

    using SystemState = std::variant<
        StateBooting,
        StateWaitingInitialization,
        StateInitializing,
        StateOperational,
        StateError,
        StateEmergencyStop,
        StatePreparePowerOff,
        StatePowerOff>;
}
