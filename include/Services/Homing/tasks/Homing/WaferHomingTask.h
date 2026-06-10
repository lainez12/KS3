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

    class WaferHomingTask : public ITask
    {
    public:
        WaferHomingTask(
            Shared<HAL::MS::IMachineStatusRepo> repo,
            Shared<HAL::Act::IPositionMotor> motor,
            Config::kinematic_profile_t fastProfile,
            Config::kinematic_profile_t fineProfile);

        void start(void) override;
        bool tick(void) override;

    protected:
        Shared<HAL::Act::IPositionMotor> m_motor;

    private:
        enum class Step
        {
            FastApproach,
            SlowApproach,
            Finished
        };

        Step m_step = Step::FastApproach;

        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Config::kinematic_profile_t m_fastProfile;
        Config::kinematic_profile_t m_fineProfile;
    };

}
