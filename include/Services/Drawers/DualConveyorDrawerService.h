#ifdef KUB_MODEL_8
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

    class DualConveyorDrawerService final : public BaseTaskService<IDrawerService>
    {
    public:
        DualConveyorDrawerService(Shared<HAL::Act::ActuatorRegistry> registry, Shared<HAL::MS::IMachineStatusRepo> repo);

        void insert(DrawerTarget target) override;
        void eject(DrawerTarget target) override;
        void stop(void) override;

    private:
        bool isWaferEjected(void);
        bool isMaskInserted(void);

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

}

#endif // KUB_MODEL_8
