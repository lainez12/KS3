#pragma once

#include <stdint.h>

namespace Kub3
{

    enum class DrawerTarget : uint32_t
    {
        None  = 0,
        Wafer = 1u << 0,
        Mask  = 1u << 1,
        Both  = Wafer | Mask
    };

    inline DrawerTarget operator&(DrawerTarget lhs, DrawerTarget rhs)
    {
        return static_cast<DrawerTarget>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }
    inline DrawerTarget operator|(DrawerTarget lhs, DrawerTarget rhs)
    {
        return static_cast<DrawerTarget>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    enum class CameraId : uint32_t
    {
        LEFT,
        RIGHT,
    };

    enum class CameraDirection : uint32_t
    {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    enum class AlignmentStageId : uint32_t
    {
        X     = 0x0,
        Y     = 0x1,
        THETA = 0x2
    };

    enum class AlignmentStageDirection : uint32_t
    {
        X_LEFT,
        X_RIGHT,
        Y_FRONT,
        Y_BACK,
        THETA_CCW,
        THETA_CW,
    };

    enum class MovementKind : uint32_t
    {
        GRANULAR,
        CONTINUOUS,
        STOP
    };

} // Kub3
