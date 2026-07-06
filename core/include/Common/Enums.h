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

    enum class AlignmentStageId : uint32_t
    {
        X,
        Y,
        THETA
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
