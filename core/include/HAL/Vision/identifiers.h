#pragma once

#include <QRect>

#define UPPER_LEFT_CAMERA  "upperLeftCamera"
#define UPPER_RIGHT_CAMERA "upperRightCamera"

namespace Kub3::HAL::Vision
{

    enum class CameraParamKind
    {
        EXPOSURE,
        GAIN,
        FRAMERATE,
        CENTERED_ZOOM,
        REGION_OF_INTEREST,
    };

    using CameraParam = std::variant<double, QRect>;

}
