#pragma once

#include <Config/machine_config.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/Actuators/Valves/IValve.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/BaseTaskService.h>
#include <Services/Homing/tasks/Homing/AlignmentStagesHomingTask.h>
#include <Services/Stowage/tasks/StowageMoveZToLimitTask.h>

#include "./IStowageService.h"

namespace Kub3::Services
{

    class StowageService final : public BaseTaskService<IStowageService>
    {
    public:
        StowageService(Shared<HAL::Act::ActuatorRegistry> registry,
                       Shared<HAL::MS::IMachineStatusRepo> repo,
                       const Config::process_config_t &config);

        void stop(void) override;
        void loadMaskToExposure(void) override;
        void loadWaferToAlignment(void) override;

    private:
        [[nodiscard]] bool isAbsoluteBottomLimitReached() const;
        [[nodiscard]] bool isAbsoluteTopLimitReached() const;

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        z_motors_bundle_t m_zMotorsBundle;
        stage_motor_bundle_t m_xMotorBundle;
        stage_motor_bundle_t m_yMotorBundle;
        stage_motor_bundle_t m_thetaMotorBundle;
        Shared<HAL::Act::IValve> m_waferVacuumValve;

        Config::process_config_t m_config;
    };

}