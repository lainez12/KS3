#include <MFSM/interlocks.h>
#include <Services/Stowage/StowageService.h>

namespace Kub3::Interlocks
{

    using namespace MFSM;

    Result canOperateDrawer(const SystemPosture &p, Services::DrawerTarget target, bool isEject)
    {
        if (p.wafer == WaferPosture::AlignmentZone || p.wafer == WaferPosture::Unknown)
            return Result::Blocked("Action Rejected: Cannot operate drawer while Wafer is stowed.");
        return Result::OK();
    }

    Result canOperateStowage(const SystemPosture &p, Services::StowageTarget target)
    {
        if (target == Services::StowageTarget::WAFER)
        {
            // Rule: Wafer cannot be stowed (raised to alignment) if Mask is not stowed (secured)
            if (p.mask != MaskPosture::Exposure)
                return Result::Blocked("Action Rejected: Secure Mask in Exposure position before raising Wafer.");
        }

        // Ensure subsystems are at least homed before moving to stowage positions
        if (p.mask == MaskPosture::Unknown || p.wafer == WaferPosture::Unknown)
            return Result::Blocked("Action Rejected: Initialization required.");

        return Result::OK();
    }

    Result canOperateUnstowage(const SystemPosture &p, Services::StowageTarget target)
    {
        if (target == Services::StowageTarget::MASK)
        {
            // Rule: Mask cannot be unstowed if Wafer is stowed (secured)
            if (p.wafer != WaferPosture::Homed)
                return Result::Blocked("Action Rejected: Unstow wafer before unstowing mask.");
        }

        // Ensure subsystems are at least homed before moving to stowage positions
        if (p.mask == MaskPosture::Unknown || p.wafer == WaferPosture::Unknown)
            return Result::Blocked("Action Rejected: Initialization required.");

        return Result::OK();
    }

    Result canEnterAlignment(const SystemPosture &p)
    {
        // Must have both components in their respective operational zones
        if (p.mask != MaskPosture::Exposure || p.wafer != WaferPosture::AlignmentZone)
            return Result::Blocked("Action Rejected: Mask and Wafer must be stowed before Alignment.");

        return Result::OK();
    }

    Result canApplyContact(const SystemPosture &p)
    {
        // Basic geometric check: Stages must be in alignment zone
        if (p.wafer != WaferPosture::AlignmentZone)
            return Result::Blocked("Action Rejected: Wafer stage is not in the Alignment Zone.");

        return Result::OK();
    }

    Result canStartExposure(const SystemPosture &p)
    {
        // Rule: Exposure prohibits Vision block movement.
        // Rule: Exposure requires Vision deck to be Homed (Safe light path)
        if (p.vision != VisionPosture::Homed)
            return Result::Blocked("Action Rejected: Move Vision deck to Home before Exposure (Optical obstruction).");

        return Result::OK();
    }

    Result canMoveAlignmentStage(const SystemPosture &p)
    {
        // Prevent X/Y/Theta movement if components are not in alignment zone
        if (p.wafer != WaferPosture::AlignmentZone)
            return Result::Blocked("Action Rejected: Stages must be in Alignment Zone for X/Y/Theta movement.");

        return Result::OK();
    }

}
