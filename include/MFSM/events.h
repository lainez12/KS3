#pragma once

#include <string>
#include <variant>

#include <HAL/Vision/identifiers.h>
#include <Services/Alignment/IAlignmentService.h>
#include <Services/Contact/IContactService.h>
#include <Services/Drawers/IDrawerService.h>
#include <Services/Vision/IVisionService.h>

// ===============================================
// Master Finite State Machine EVENTS Definitions
// ===============================================

namespace Kub3::MFSM
{

    // Emitted when software has successfully connected to the hardware
    struct EvHardwareReady {};

    struct EvHardwareError {
        std::string reason;
    };

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

    struct CmdCameraParamUpdate {
        QString cameraId;
        HAL::Vision::CameraParamKind kind;
        HAL::Vision::CameraParam value;
    };

    struct CmdEnterAlignmentMode {
        bool autoMode = false;
    };

    struct CmdExitAlignmentMode {};

    struct CmdAlignmentPad {
        Services::AlignmentStage targetStage;
        Services::AlignmentPayload operation;
    };

    struct CmdZAxisPad {
        Services::ZAxisPayload operation;
    };

    struct CmdVisionPad {
        Services::VisionMotor targetMotor;
        Services::VisionPayload operation;
    };

    struct CmdStartAutolevel {};

    struct CmdApplyContact {
        double forceGF;
    };

    // Internal Service Events

    struct EvServiceSuccess {};

    struct EvServiceError {
        std::string reason;
    };

    struct EvEmergencyStopTriggered {
        std::string reason;
    };

    struct EvContactSequenceComplete {};

    struct EvPowerOff {};

    using SystemEvent = std::variant<
        // Boot & Initialization events
        EvHardwareReady,
        EvHardwareError,
        EvInitializationComplete,
        // User commands
        CmdResetError,
        CmdStartInitialization,
        // --- Drawers/Conveyors
        CmdOperateDrawer,
        // --- Vision settings
        CmdCameraParamUpdate,
        // --- Horizontality & Contact
        CmdStartAutolevel,
        CmdApplyContact,
        // --- Alignment
        CmdEnterAlignmentMode,
        CmdExitAlignmentMode,
        CmdAlignmentPad,
        CmdZAxisPad,
        CmdVisionPad,
        // Internal & Services events
        EvServiceSuccess,
        EvServiceError,
        EvEmergencyStopTriggered,
        EvContactSequenceComplete,
        // Shutdown / power off
        EvPowerOff>;

} // namespace Kub3::MFSM
