#ifndef KUB_MODEL_8
#pragma once

#include <QElapsedTimer>
#include <memory>

#include "HAL/Actuators/ActuatorRegistry.h"
#include "HAL/MachineStatus/IMachineStatusRepo.h"
#include "IDrawerService.h"
#include "Services/BaseTaskService.h"
#include "utils.h"

namespace Kub3::Services
{

    // TODO: add process config as ctor parameter
    class SingleConveyorDrawerService final : public BaseTaskService<IDrawerService>
    {
    public:
        SingleConveyorDrawerService(Shared<HAL::Act::ActuatorRegistry> registry, Shared<HAL::MS::IMachineStatusRepo> repo);

        void insert(DrawerTarget target) override;
        void eject(DrawerTarget target) override;
        void stop(void) override;

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

}

#endif // not KUB_MODEL_8
