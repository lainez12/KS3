#pragma once

#include "HAL/MachineStatus/IMachineStatusRepo.h"
#include <utils.h>

namespace Kub3::HAL::MS
{
    bool readBool(const Shared<IMachineStatusRepo> &repo, const std::string &key);
}
