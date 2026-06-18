#pragma once

#include <string>

#include <Common/Enums.h>
#include <Services/IService.h>

#define DRAWER_PROCEDURE_TIMEOUT_MS 30000

namespace Kub3::Services
{

    class IDrawerService : public IService
    {
    public:
        virtual ~IDrawerService() = default;

        // Macro-commands (for MFSM)
        virtual void eject(DrawerTarget target = DrawerTarget::Both)  = 0;
        virtual void insert(DrawerTarget target = DrawerTarget::Both) = 0;
    };

}
