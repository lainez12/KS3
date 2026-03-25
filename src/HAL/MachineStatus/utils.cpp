#include "HAL/MachineStatus/utils.h"

namespace Kub3::HAL::MS
{
    bool readBool(const Shared<Kub3::HAL::MS::IMachineStatusRepo> &repo, const std::string &key)
    {
        Optional<bool> optVal = repo->getSensor<bool>(key);

        return optVal.value_or(false);
    }
}
