#include <Common/Enums.h>
#include <MFSM/interlocks.h>
#include <Services/Stowage/StowageService.h>

namespace Kub3::Interlocks
{

    using namespace MFSM;

    Result<Unit, const char *> canOperateDrawer(const SystemPosture &p, DrawerTarget target, bool isEject)
    {
        if (p.wafer == WaferPosture::AlignmentZone || p.wafer == WaferPosture::Unknown)
            return Err("Action Rejected: Cannot operate drawer while Wafer is stowed.");
        return Ok<Unit>({});
    }

    Result<Unit, const char *> canOperateStowage(const SystemPosture &p, Services::StowageTarget target)
    {
        if (target == Services::StowageTarget::WAFER)
        {
            // Rule: Wafer cannot be stowed (raised to alignment) if Mask is not stowed (secured)
            if (p.mask != MaskPosture::Exposure)
                return Err("Action Rejected: Secure Mask in Exposure position before raising Wafer.");
        }

        // Ensure subsystems are at least homed before moving to stowage positions
        if (p.mask == MaskPosture::Unknown || p.wafer == WaferPosture::Unknown)
            return Err("Action Rejected: Initialization required.");

        return Ok<Unit>({});
    }

    Result<Unit, const char *> canOperateUnstowage(const SystemPosture &p, Services::StowageTarget target)
    {
        if (target == Services::StowageTarget::MASK)
        {
            // Rule: Mask cannot be unstowed if Wafer is stowed (secured)
            if (p.wafer != WaferPosture::Homed)
                return Err("Action Rejected: Unstow wafer before unstowing mask.");
        }

        // Ensure subsystems are at least homed before moving to stowage positions
        if (p.mask == MaskPosture::Unknown || p.wafer == WaferPosture::Unknown)
            return Err("Action Rejected: Initialization required.");

        return Ok<Unit>({});
    }

    Result<Unit, const char *> canEnterAlignment(const SystemPosture &p)
    {
        // Must have both components in their respective operational zones
        if (p.mask != MaskPosture::Exposure || p.wafer != WaferPosture::AlignmentZone)
            return Err("Action Rejected: Mask and Wafer must be stowed before Alignment.");

        return Ok<Unit>({});
    }

    Result<Unit, const char *> canApplyContact(const SystemPosture &p)
    {
        // Basic geometric check: Stages must be in alignment zone
        if (p.wafer != WaferPosture::AlignmentZone)
            return Err("Action Rejected: Wafer stage is not in the Alignment Zone.");

        return Ok<Unit>({});
    }

    Result<Unit, const char *> canStartExposure(const SystemPosture &p)
    {
        // Rule: Exposure prohibits Vision block movement.
        // Rule: Exposure requires Vision deck to be Homed (Safe light path)
        if (p.vision != VisionPosture::Homed)
            return Err("Action Rejected: Move Vision deck to Home before Exposure (Optical obstruction).");

        return Ok<Unit>({});
    }

    Result<Unit, const char *> canMoveAlignmentStage(const SystemPosture &p)
    {
        // Prevent X/Y/Theta movement if components are not in alignment zone
        if (p.wafer != WaferPosture::AlignmentZone)
            return Err("Action Rejected: Stages must be in Alignment Zone for X/Y/Theta movement.");

        return Ok<Unit>({});
    }

}
