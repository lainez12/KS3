#pragma once

#include <stdint.h>

namespace Kub3
{

    enum class DrawerTarget
    {
        Wafer,
        Mask,
        Both
    };

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

    enum class CameraMovementKind : uint32_t
    {
        GRANULAR,
        CONTINUOUS,
        STOP
    };

} // Kub3
