#pragma once

#include <QObject>

#include "HAL/Actuators/Motors/IMotor.h"
#include "HAL/MachineStatus/IMachineStatusRepo.h"
#include "HAL/MachineStatus/actuators_labels.h"
#include "HAL/MachineStatus/sensors_labels.h"
#include "Services/ITask.h"
#include "utils.h"

namespace Kub3::Services
{

    class WaferInsertionTask final : public ITask
    {
    public:
        WaferInsertionTask(
            Shared<HAL::Act::IMotor> motor,
            Shared<HAL::MS::IMachineStatusRepo> repo,
            Config::kinematic_profile_t fastProfile,
            Config::kinematic_profile_t fineProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        enum class Step
        {
            FastApproach,
            SlowApproach,
            Finished
        };

        Step m_step = Step::FastApproach;

        Shared<HAL::Act::IMotor> m_motor;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Config::kinematic_profile_t m_fastProfile;
        Config::kinematic_profile_t m_fineProfile;
    };

}
