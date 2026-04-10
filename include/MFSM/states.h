#pragma once

#include <string>
#include <variant>

#include "Services/Drawers/IDrawerService.h"
#include "events.h"

// ===============================================
// Master Finite State Machine STATES Definitions
// ===============================================

namespace Kub3::MFSM
{

    namespace Payloads
    {
        // ---------------
        // --- PAYLOADS
        // ---------------

        struct DrawerOpPayload {
            DrawerOperation kind;          // Insert/Eject
            Services::DrawerTarget target; // Wafer/Mask/Both
        };

        using OperatingPayload = std::variant<
            DrawerOpPayload
            // TODO: add more kinds of payload
            >;

    } // namespace Payloads

    // -------------
    // --- STATES
    // -------------

    enum class ProcessServiceKind
    {
        DRAWER
    };

    struct StateBooting {
        uint32_t ticksElapsed = 0;
        // Keeps track of how many times each specific subsystem has been retried
        std::unordered_map<std::string, int8_t> retryCounts;
    };

    struct StateWaitingInitialization {};

    struct StateInitialization {};

    struct StateIdle {};

    struct StateOperating {
        Payloads::OperatingPayload payload;
    };

    struct StateError {
        std::string message;
    };

    struct StateEmergencyStop {
        std::string reason;
    };

    // The Single Source of Truth for the Machine's High-Level State
    using SystemState = std::variant<
        StateBooting,
        StateWaitingInitialization,
        StateInitialization,
        StateIdle,
        StateOperating,
        StateError,
        StateEmergencyStop>;

    // ---------------------------------------
    // --- C++20 OVERLOADED VISITOR HELPER
    // ---------------------------------------
    template <class... Ts>
    struct overloadedCallable : Ts... {
        using Ts::operator()...;
    };
    // Deduction guide for compiler (Not strictly required in C++20 as type inference is smart, but good practice)
    template <class... Ts>
    overloadedCallable(Ts...) -> overloadedCallable<Ts...>;

} // namespace Kub3::MFSM
