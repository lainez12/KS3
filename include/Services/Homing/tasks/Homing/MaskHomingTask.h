#pragma once

#include <QObject>

#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <Services/ITask.h>
#include <utils.h>

namespace Kub3::Services
{

    class MaskHomingTask : public ITask
    {
    public:
        MaskHomingTask(Shared<HAL::MS::IMachineStatusRepo> repo,
                       Shared<HAL::Act::IPositionMotor> motor,
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

    protected:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Shared<HAL::Act::IPositionMotor> m_motor;

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

        Config::kinematic_profile_t m_fastProfile;
        Config::kinematic_profile_t m_fineProfile;
        Config::kinematic_profile_t m_contactProfile;
    };

}
