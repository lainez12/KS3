#pragma once

#include <QObject>

#include "HAL/Actuators/IMotor.h"
#include "HAL/MachineStatus/IMachineStatusRepo.h"
#include "HAL/MachineStatus/actuators_labels.h"
#include "HAL/MachineStatus/sensors_labels.h"
#include "Services/ITask.h"
#include "utils.h"

namespace Kub3::Services
{

    class WaferEjectionTask final : public ITask
    {
    public:
        WaferEjectionTask(Shared<HAL::Act::IMotor> motor, Shared<HAL::MS::IMachineStatusRepo> repo);

        void start(void) override;
        bool tick(void) override;

    private:
        enum class Step
        {
            FastApproach,
            SlowApproach, // TODO: use with encoder position condition
            Finished
        };

        Step m_step = Step::FastApproach;

        Shared<HAL::Act::IMotor> m_motor;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

}
