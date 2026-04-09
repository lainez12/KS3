#include <Algorithms/Kinematic/utils.h>

namespace Kub3::Algorithms::Kinematic
{

    Unique<IKinematicGenerator> buildKinematicGenerator(KinematicGeneratorKind kind)
    {
        if (kind < 0 || kind >= KinematicGeneratorKind::TYPES_COUNT)
            throw std::out_of_range(std::format("Invalid KinematicGeneratorKind: {}", static_cast<int>(kind)));

        return kinematicsGenBuilders[kind]();
    }

}
