#pragma once

#include <Services/IService.h>

namespace Kub3::Services
{

    enum StowageTarget : uint32_t;

    class IStowageService : public IService
    {
    public:
        virtual ~IStowageService()                      = default;
        virtual void startStowage(StowageTarget target) = 0;
    };

}