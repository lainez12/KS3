#include <QDebug>

#include "HAL/MachineStatus/utils.h"

namespace Kub3::HAL::MS
{

    bool readBool(const Shared<Kub3::HAL::MS::IMachineStatusRepo> &repo, const std::string &key)
    {
        Optional<bool> optVal = repo->getSensor<bool>(key);

        if (!optVal.has_value())
        {
            qCritical().nospace() << "[CRITICAL] Unable to get sensor value of type 'bool' for key: " << key;
            return false;
        }

        return optVal.value();
    }

    int32_t readInt32(const Shared<Kub3::HAL::MS::IMachineStatusRepo> &repo, const std::string &key)
    {
        Optional<int32_t> optVal = repo->getSensor<int32_t>(key);

        if (!optVal.has_value())
        {
            qCritical().nospace() << "[CRITICAL] Unable to get sensor value of type 'int32_t' for key: " << key;
            return INT32_MAX;
        }

        return optVal.value();
    }

    uint32_t readUInt32(const Shared<Kub3::HAL::MS::IMachineStatusRepo> &repo, const std::string &key)
    {
        Optional<uint32_t> optVal = repo->getSensor<uint32_t>(key);

        if (!optVal.has_value())
        {
            qCritical().nospace() << "[CRITICAL] Unable to get sensor value of type 'uint32_t' for key: " << key;
            return INT32_MAX;
        }

        return optVal.value();
    }

    uint16_t readUInt16(const Shared<Kub3::HAL::MS::IMachineStatusRepo> &repo, const std::string &key)
    {
        Optional<uint16_t> optVal = repo->getSensor<uint16_t>(key);

        if (!optVal.has_value())
        {
            qCritical().nospace() << "[CRITICAL] Unable to get sensor value of type 'uint16_t' for key: " << key;
            return UINT16_MAX;
        }

        return optVal.value();
    }

}
