#pragma once

#include <MFSM/posture.h>
#include <Services/Drawers/IDrawerService.h>
#include <Services/Stowage/IStowageService.h>

namespace Kub3::Interlocks
{
    struct Result {
        bool allowed;
        const char *message; // UI-friendly explanation

        explicit operator bool() const
        {
            return allowed;
        }
        static Result OK()
        {
            return {true, nullptr};
        }
        static Result Blocked(const char *msg)
        {
            return {false, msg};
        }
    };

    // --- Drawer Interlocks ---
    Result canOperateDrawer(const MFSM::SystemPosture &p, Services::DrawerTarget target, bool isEject);

    // --- Stowage Interlocks ---
    Result canOperateStowage(const MFSM::SystemPosture &p, Services::StowageTarget target);
    Result canOperateUnstowage(const MFSM::SystemPosture &p, Services::StowageTarget target);

    // --- Mode & Sequence Interlocks ---
    Result canEnterAlignment(const MFSM::SystemPosture &p);
    Result canExitAlignment(const MFSM::SystemPosture &p);
    Result canApplyContact(const MFSM::SystemPosture &p);
    Result canStartExposure(const MFSM::SystemPosture &p);

    // --- Movement Pads ---
    Result canMoveAlignmentStage(const MFSM::SystemPosture &p);
}
