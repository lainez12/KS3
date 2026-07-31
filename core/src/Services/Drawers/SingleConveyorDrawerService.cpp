#if defined(KUB_MODEL_4) || defined(KUB_MODEL_6)

// HAL
#include "HAL/Actuators/Motors/IMotor.h"
#include "HAL/MachineStatus/actuators_labels.h"
#include "HAL/MachineStatus/sensors_labels.h"
#include "HAL/MachineStatus/utils.h"
// Services
#include "Services/Drawers/SingleConveyorDrawerService.h"

namespace Kub3::Services
{

    SingleConveyorDrawerService::SingleConveyorDrawerService(Shared<HAL::Act::ActuatorRegistry> registry, Shared<HAL::MS::IMachineStatusRepo> repo) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo))
    {
    }

    void SingleConveyorDrawerService::insert(DrawerTarget target)
    {
        throw std::runtime_error("Not implemented");
        this->clearTasks();
        this->startSequence();
    }

    void SingleConveyorDrawerService::eject(DrawerTarget target)
    {
        throw std::runtime_error("Not implemented");
        this->clearTasks();
        this->startSequence();
    }

    void SingleConveyorDrawerService::stop(void)
    {
        throw std::runtime_error("Not implemented");
        BaseTaskService::stop();
    }

}

#endif // defined(KUB_MODEL_4) || defined(KUB_MODEL_6)
