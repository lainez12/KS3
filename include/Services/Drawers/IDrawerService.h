#pragma once

#include <string>

#include "Services/IService.h"

#define DRAWER_PROCEDURE_TIMEOUT_MS 30000

namespace Kub3::Services
{

    enum class DrawerTarget
    {
        Wafer,
        Mask,
        Both
    };

    enum class DrawerStatus
    {
        Unknown,
        Opened,
        Closed,
        Moving,
        Error
    };

    class IDrawerService : public IService
    {
    public:
        virtual ~IDrawerService() = default;

        // Macro-commands (for MFSM)
        virtual void eject(DrawerTarget target = DrawerTarget::Both)  = 0;
        virtual void insert(DrawerTarget target = DrawerTarget::Both) = 0;
    };

}
