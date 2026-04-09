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

    typedef Unique<IKinematicGenerator> (*kinematic_generator_builder)(void);

    const kinematic_generator_builder kinematicsGenBuilders[KinematicGeneratorKind::TYPES_COUNT] = {
        [KinematicGeneratorKind::TRAPEZOIDAL] = []() -> Unique<IKinematicGenerator> { return std::move(std::make_unique<TrapezoidalGenerator>()); },
    };

    Unique<IKinematicGenerator> buildKinematicGenerator(KinematicGeneratorKind kind)
    {
        if (kind < 0 || kind >= TYPES_COUNT)
            throw std::out_of_range(std::format("Invalid KinematicGeneratorKind: {}", kind));

        return kinematicsGenBuilders[kind]();
    }

}
