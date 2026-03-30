#pragma once

#include "HAL/MachineStatus/IMachineStatusRepo.h"
#include <utils.h>

namespace Kub3::HAL::MS
{
    bool readBool(const Shared<IMachineStatusRepo> &repo, const std::string &key);
    int32_t readInt(const Shared<Kub3::HAL::MS::IMachineStatusRepo> &repo, const std::string &key);
    uint32_t readUInt(const Shared<Kub3::HAL::MS::IMachineStatusRepo> &repo, const std::string &key);
}
