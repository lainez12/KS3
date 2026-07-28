#include <Common/Enums.h>
#include <MFSM/interlocks.h>
#include <Services/Stowage/StowageService.h>

#define OK Ok<Unit>({})

namespace Kub3::Interlocks
{

    using namespace MFSM;

    Result<Unit, const char *> canOperateDrawer(const SystemPosture &p, DrawerTarget target, bool isEject)
    {
        if (p.wafer == WaferPosture::Unknown)
            return Err("Action Rejected - Cannot operate drawer while Wafer is in the following posture: UNKNOWN");
        if (p.wafer == WaferPosture::ElevatorMidway)
            return Err("Action Rejected - Cannot operate drawer while Wafer is in the following posture: ELEVATORS MIDWAY");
        if (p.wafer == WaferPosture::AlignmentZone)
            return Err("Action Rejected - Cannot operate drawer while Wafer is in the following posture: ALIGNMENT ZONE");

        return OK;
    }

    Result<Unit, const char *> canOperateStowage(const SystemPosture &p, StowageTarget target)
    {
        if (target == StowageTarget::None)
        {
            return Err("Action Rejected - Invalid payload. Cannot stow nothing.");
        }

        if (has_flag(target, StowageTarget::Wafer))
        {
            // Rule: Wafer cannot be stowed (raised to alignment) if Mask is not stowed (secured)
            if (p.mask != MaskPosture::Exposure)
                return Err("Action Rejected - Cannot stow Wafer while mask is not in exposure position.");
            // Invalid wafer stowage states
            if (p.wafer == WaferPosture::Unknown)
                return Err("Action Rejected - Cannot perform stowage while Wafer is in the following posture: UNKNOWN");
            if (p.wafer == WaferPosture::Ejected)
                return Err("Action Rejected - Cannot perform stowage while Wafer is in the following posture: EJECTED");
            if (p.wafer == WaferPosture::DrawerMidway)
                return Err("Action Rejected - Cannot perform stowage while Wafer is in the following posture: DRAWER MIDWAY");
        }

        if (has_flag(target, StowageTarget::Mask))
        {
            if (p.mask == MaskPosture::Unknown)
                return Err("Action Rejected - Cannot perform stowage while Mask is in the following posture: UNKNOWN");
        }

        return OK;
    }

    Result<Unit, const char *> canOperateUnstowage(const SystemPosture &p, StowageTarget target)
    {
        if (has_flag(target, StowageTarget::Mask))
        {
            // Rule: Mask cannot be unstowed if Wafer is stowed (secured)
            if (p.wafer == WaferPosture::ElevatorMidway)
                return Err("Action Rejected - Cannot unstow Mask while Wafer is in the following posture: ELEVATOR MIDWAY");
            if (p.wafer == WaferPosture::AlignmentZone)
                return Err("Action Rejected - Cannot unstow Mask while Wafer is in the following posture: ALIGNMENT ZONE");
            // Invalid mask unstowage states
            if (p.mask == MaskPosture::Ejected)
                return Err("Action Rejected - Cannot unstow Mask while it is in the following posture: EJECTED");
            if (p.mask == MaskPosture::DrawerMidway)
                return Err("Action Rejected - Cannot unstow Mask while it is in the following posture: DRAWER MIDWAY");
            if (p.mask == MaskPosture::Homed)
                return Err("Action Rejected - Cannot unstow Mask while it is in the following posture: HOMED");
            if (p.mask == MaskPosture::Unknown)
                return Err("Action Rejected - Cannot unstow Mask while it is in the following posture: UNKNOWN");
        }

        if (has_flag(target, StowageTarget::Wafer))
        {
            if (p.wafer == WaferPosture::Ejected)
                return Err("Action Rejected - Cannot unstow Wafer while it is in the following posture: EJECTED");
            if (p.wafer == WaferPosture::DrawerMidway)
                return Err("Action Rejected - Cannot unstow Wafer while it is in the following posture: DRAWER MIDWAY");
            if (p.wafer == WaferPosture::Homed)
                return Err("Action Rejected - Cannot unstow Wafer while it is in the following posture: HOMED");
            if (p.wafer == WaferPosture::Unknown)
                return Err("Action Rejected - Cannot unstow Wafer while it is in the following posture: UNKNOWN");
        }

        return OK;
    }

    Result<Unit, const char *> canOperateAutolevel(const SystemPosture &p)
    {
        if (p.mask != MaskPosture::Exposure)
            return Err("Action Rejected - Unable to autolevel while the mask is not stowed in exposure position");
        if (p.wafer != WaferPosture::AlignmentZone)
            return Err("Action Rejected - Unable to autolevel while the wafer is not in alignment zone");
        return OK;
    }

    Result<Unit, const char *> canEnterAlignment(const SystemPosture &p)
    {
        // Must have both components in their respective operational zones
        if (p.mask != MaskPosture::Exposure || p.wafer != WaferPosture::AlignmentZone)
            return Err("Action Rejected - Mask and Wafer must be stowed before Alignment.");

        return OK;
    }

    Result<Unit, const char *> canApplyContact(const SystemPosture &p)
    {
        // Basic geometric check: Stages must be in alignment zone
        if (p.wafer != WaferPosture::AlignmentZone)
            return Err("Action Rejected - Wafer stage is not in the Alignment Zone.");
        // Mask must be stowed
        if (p.mask != MaskPosture::Exposure)
            return Err("Action Rejected - Mask is not in the required posture: EXPOSURE");

        return OK;
    }

    Result<Unit, const char *> canStartExposure(const SystemPosture &p)
    {
        // Rule: Exposure prohibits Vision block movement.
        // Rule: Exposure requires Vision deck to be Homed (Safe light path)
        if (p.vision != VisionPosture::Homed)
            return Err("Action Rejected - Move Vision deck to Home before Exposure (Optical obstruction).");

        return OK;
    }

    Result<Unit, const char *> canMoveAlignmentStage(const SystemPosture &p)
    {
        // Prevent X/Y/Theta movement if components are not in alignment zone
        if (p.wafer != WaferPosture::AlignmentZone)
            return Err("Action Rejected - Stages must be in Alignment Zone for X/Y/Theta movement.");

        return OK;
    }

}

#undef OK
