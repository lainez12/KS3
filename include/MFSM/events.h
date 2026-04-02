#pragma once

#include <string>
#include <variant>

#include "Services/Drawers/IDrawerService.h"

// ===============================================
// Master Finite State Machine EVENTS Definitions
// ===============================================

namespace Kub3::MFSM
{

    // Emitted when software has successfully connected to the hardware
    struct EvHardwareReady {};

    // Emitted when software has successfully performed the initialization routine
    struct EvInitializationComplete {};

    // UI commands

    enum class DrawerOperation
    {
        INSERT,
        EJECT
    };

    struct CmdStartInitialization {};

    struct CmdOperateDrawer {
        Services::DrawerTarget target;
        DrawerOperation operation;
    };

    struct CmdResetError {};

    // Internal Service Events

    struct EvServiceSuccess {};

    struct EvServiceError {
        std::string reason;
    };

    struct EvEmergencyStopTriggered {
        std::string reason;
    };

    using SystemEvent = std::variant<
        // Boot & Initialization events
        EvHardwareReady,
        EvInitializationComplete,
        // UI commands
        CmdStartInitialization,
        CmdOperateDrawer,
        CmdResetError,
        // Internal Service Event
        EvServiceSuccess,
        EvServiceError,
        EvEmergencyStopTriggered>;

} // namespace Kub3::MFSM
