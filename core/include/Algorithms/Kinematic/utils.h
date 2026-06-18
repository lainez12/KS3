#pragma once

#include <format>

#include "IKinematicGenerator.h"
#include "TrapezoidalGenerator.h"
#include <utils.h>

namespace Kub3::Algorithms::Kinematic
{

    enum KinematicGeneratorKind
    {
        TRAPEZOIDAL = 0,
        TYPES_COUNT // DO NOT USE AS KEY FOR THE BUILDERS MAP
    };

    Unique<IKinematicGenerator> buildKinematicGenerator(KinematicGeneratorKind kind);

}
