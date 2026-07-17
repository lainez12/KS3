#pragma once

#include <stdint.h>

#define impl_bin_operators_for_enum(EnumClass)                                                  \
    inline EnumClass operator&(EnumClass lhs, EnumClass rhs)                                    \
    {                                                                                           \
        return static_cast<EnumClass>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); \
    }                                                                                           \
    inline EnumClass operator|(EnumClass lhs, EnumClass rhs)                                    \
    {                                                                                           \
        return static_cast<EnumClass>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
    }

namespace Kub3
{

    enum class DrawerTarget : uint32_t
    {
        None  = 0,
        Wafer = 1u << 0,
        Mask  = 1u << 1,
        Both  = Wafer | Mask
    };
    impl_bin_operators_for_enum(DrawerTarget);

    enum class ForceSensor : uint32_t
    {
        None  = 0,
        Left  = 1u << 0,
        Right = 1u << 1,
        Back  = 1u << 2,
        All   = Left | Right | Back,
    };
    impl_bin_operators_for_enum(ForceSensor);

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
