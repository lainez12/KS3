#pragma once

#include <Services/IService.h>

namespace Kub3::Services
{

    enum class StowageTarget
    {
        WAFER,
        MASK
    };

    class IStowageService : public IService
    {
    public:
        virtual ~IStowageService()              = default;
        virtual void loadMaskToExposure(void)   = 0;
        virtual void loadWaferToAlignment(void) = 0;
    };

}