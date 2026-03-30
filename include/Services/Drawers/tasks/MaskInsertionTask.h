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

    class MaskInsertionTask final : public ITask
    {
    public:
        MaskInsertionTask(
            Shared<HAL::Act::IMotor> motor,
            Shared<HAL::MS::IMachineStatusRepo> repo,
            Config::kinematic_profile_t fastProfile,
            Config::kinematic_profile_t fineProfile,
            Config::kinematic_profile_t contactProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        void fastApproachLogic(void);
        void slowApproachLogic(void);
        void reverseClearanceLogic(void);
        void contactModeLogic(void);

    private:
        enum class Step
        {
            FastApproach,
            SlowApproach,
            ReverseClearance,
            ContactMode,
            Finished
        };

        Step m_step = Step::FastApproach;

        Shared<HAL::Act::IMotor> m_motor;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Config::kinematic_profile_t m_fastProfile;
        Config::kinematic_profile_t m_fineProfile;
        Config::kinematic_profile_t m_contactProfile;
    };

}
