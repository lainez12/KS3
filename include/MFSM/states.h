#pragma once

#include <string>
#include <variant>

#include <Services/Drawers/IDrawerService.h>
#include <Services/Homing/IHomingService.h>
#include <utils.h>

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

        struct HomingOpPayload {
            Services::HomingTarget::Type target;
        };

        // Represents the mechanical phase of the alignment process
        enum class AlignmentPhase
        {
            Free,            // X, Y, Theta OK | Cameras OK | Z Manual OK
            ApplyingContact, // X, Y, Theta LOCKED | Cameras OK | Z Automated ONLY
            InContact,       // X, Y, Theta LOCKED | Cameras OK | Z Manual OK
            Separating       // X, Y, Theta LOCKED | Cameras OK | Z Automated ONLY
        };

        struct AlignmentOpPayload {
            AlignmentPhase phase = AlignmentPhase::Free;
            bool isAutoAlignment; // TODO: Implement when creating the automatic alignment algorithm
        };

        using OperatingPayload = std::variant<
            DrawerOpPayload,
            HomingOpPayload,
            AlignmentOpPayload>;

    } // namespace Payloads

    // -------------
    // --- STATES
    // -------------

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

    struct StatePowerOff {};

    // The Single Source of Truth for the Machine's High-Level State
    using SystemState = std::variant<
        StateBooting,
        StateWaitingInitialization,
        StateInitialization,
        StateIdle,
        StateOperating,
        StateError,
        StateEmergencyStop,
        StatePowerOff>;

} // namespace Kub3::MFSM
