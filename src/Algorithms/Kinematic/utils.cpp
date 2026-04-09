#include <Algorithms/Kinematic/utils.h>

namespace Kub3::Algorithms::Kinematic
{

    typedef Unique<IKinematicGenerator> (*kinematic_generator_builder)(void);

    static const kinematic_generator_builder kinematicsGenBuilders[KinematicGeneratorKind::TYPES_COUNT] = {
        [KinematicGeneratorKind::TRAPEZOIDAL] = []() -> Unique<IKinematicGenerator> { return std::move(std::make_unique<TrapezoidalGenerator>()); },
    };

    Unique<IKinematicGenerator> buildKinematicGenerator(KinematicGeneratorKind kind)
    {
        if (kind < 0 || kind >= KinematicGeneratorKind::TYPES_COUNT)
            throw std::out_of_range(std::format("Invalid KinematicGeneratorKind: {}", static_cast<int>(kind)));

        return kinematicsGenBuilders[kind]();
    }

}
