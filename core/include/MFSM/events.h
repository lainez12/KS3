#pragma once

#include <string>
#include <variant>

#include <Common/Enums.h>
#include <HAL/Vision/identifiers.h>
#include <Services/Alignment/IAlignmentService.h>
#include <Services/Contact/IContactService.h>
#include <Services/Drawers/IDrawerService.h>
#include <Services/Exposure/IExposureService.h>
#include <Services/Stowage/IStowageService.h>
#include <Services/Vision/IVisionService.h>
#include <utils.h>

// ===============================================
// Master Finite State Machine EVENTS Definitions
// ===============================================

namespace Kub3::MFSM
{

    // =============================
    // MACRO EVENTS
    // =============================

    // --- BOOT & INITIALIZATION EVENTS ---

    // Emitted when software has successfully connected to the hardware
    struct EvHardwareReady {};
    struct EvHardwareError {
        std::string reason;
    };
    // Emitted when software has successfully performed the initialization routine
    struct EvInitializationComplete {};

    // --- MACRO SYSTEM COMMANDS ---

    struct CmdRetryBoot {};
    struct CmdStartInitialization {};
    struct CmdAbortOperation {};
    struct CmdResetError {};
    struct EvEmergencyStopTriggered {
        std::string reason;
    };
    struct EvPowerOff {};

    // =============================
    // OPERATIONAL COMMANDS
    // =============================

    // --- DRAWER & STOWAGE COMMANDS ---

    enum class DrawerOperation
    {
        INSERT,
        EJECT
    };
    struct CmdOperateDrawer {
        DrawerTarget target;
        DrawerOperation operation;
    };
    struct CmdOperateStowage {
        StowageTarget target;
    };
    struct CmdOperateUnstowage {
        StowageTarget target;
    };

    // --- ALIGNMENT & CONTACT COMMANDS ---

    struct CmdEnterAlignmentMode {
        bool autoMode = false;
    };
    struct CmdExitAlignmentMode {};
    struct CmdStartAutolevel {};
    struct CmdApplyContact {
        double forceGF;
    };

    // --- MANUAL PAD MOVEMENTS (Interactive) ---

    struct CmdAlignmentPad {
        AlignmentStageId stageId;
        Services::AlignmentPayload operation;
    };
    struct CmdZAxisPad {
        Services::ZAxisPayload operation;
    };
    struct CmdVisionPad {
        Services::VisionMotor targetMotor;
        Services::VisionPayload operation;
    };

    // --- VISION & EXPOSURE COMMANDS ---

    struct CmdEnterExposureMode {};
    struct CmdCameraParamUpdate {
        QString cameraId;
        HAL::Vision::CameraParamKind kind;
        HAL::Vision::CameraParam value;
    };
    struct CmdStartExposure {
        Services::ExposurePayload payload;
    };

    // ===============================
    // INTERNAL SERVICE FEEDBACK
    // ===============================

    struct EvServiceSuccess {};
    struct EvServiceError {
        std::string reason;
    };
    struct EvContactSequenceComplete {};

    // ===============================
    // SYSTEM EVENT VARIANT
    // ===============================
    using SystemEvent = std::variant<
        // Boot & Initialization events
        EvHardwareReady,
        EvHardwareError,
        EvInitializationComplete,

        // User commands
        CmdResetError,
        CmdRetryBoot,
        CmdStartInitialization,
        CmdAbortOperation,
        // --- Drawers/Conveyors
        CmdOperateDrawer,
        // --- Stowage (Wafer holder securing / Mask to exposure)
        CmdOperateStowage,
        CmdOperateUnstowage,
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
        // --- Exposure
        CmdStartExposure,

        // Internal & Services events
        EvServiceSuccess,
        EvServiceError,
        EvEmergencyStopTriggered,
        EvContactSequenceComplete,
        // Shutdown / power off
        EvPowerOff>;

} // namespace Kub3::MFSM
