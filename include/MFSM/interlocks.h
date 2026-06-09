#pragma once

#include <Common/Enums.h>
#include <Common/Result.h>
#include <MFSM/posture.h>
#include <Services/Drawers/IDrawerService.h>
#include <Services/Stowage/IStowageService.h>

namespace Kub3::Interlocks
{
    // struct Result {
    //     bool allowed;
    //     const char *message; // UI-friendly explanation

    //     explicit operator bool() const
    //     {
    //         return allowed;
    //     }
    //     static Result OK()
    //     {
    //         return {true, nullptr};
    //     }
    //     static Result Blocked(const char *msg)
    //     {
    //         return {false, msg};
    //     }
    // };

    // --- Drawer Interlocks ---
    Result<Unit, const char *> canOperateDrawer(const MFSM::SystemPosture &p, DrawerTarget target, bool isEject);

    // --- Stowage Interlocks ---
    Result<Unit, const char *> canOperateStowage(const MFSM::SystemPosture &p, Services::StowageTarget target);
    Result<Unit, const char *> canOperateUnstowage(const MFSM::SystemPosture &p, Services::StowageTarget target);

    // --- Mode & Sequence Interlocks ---
    Result<Unit, const char *> canEnterAlignment(const MFSM::SystemPosture &p);
    Result<Unit, const char *> canExitAlignment(const MFSM::SystemPosture &p);
    Result<Unit, const char *> canApplyContact(const MFSM::SystemPosture &p);
    Result<Unit, const char *> canStartExposure(const MFSM::SystemPosture &p);

    // --- Movement Pads ---
    Result<Unit, const char *> canMoveAlignmentStage(const MFSM::SystemPosture &p);
}
