#pragma once

#include <Common/Enums.h>
#include <Services/IService.h>

namespace Kub3::Services
{

    class IStowageService : public IService
    {
    public:
        virtual ~IStowageService()                      = default;
        virtual void startStowage(StowageTarget target) = 0;
    };

}