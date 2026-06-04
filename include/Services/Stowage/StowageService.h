#pragma once

#include <Config/conf.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/Actuators/Valves/IValve.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/Sensors/ISensor.h>
#include <Services/BaseTaskService.h>
#include <Services/Homing/tasks/Homing/AlignmentStagesHomingTask.h>
#include <Services/Stowage/tasks/StowageMoveZToLimitTask.h>

#include "./IStowageService.h"

namespace Kub3::Services
{

    enum StowageTarget : uint32_t
    {
        MASK  = 0x1 << 0,
        WAFER = 0x1 << 1,
        BOTH  = MASK & WAFER,
    };

    class StowageService final : public BaseTaskService<IStowageService>
    {
    public:
        StowageService(Shared<HAL::Act::ActuatorRegistry> registry,
                       Shared<HAL::MS::IMachineStatusRepo> repo,
                       const Config::process_config_t &config);

        void startStowage(StowageTarget target) override;
        void stop(void) override;

    private:
        bool buildMaskStowageTaskQueue(void);
        bool buildWaferStowageTaskQueue(void);

    private:
        [[nodiscard]] bool isAbsoluteBottomLimitReached() const;
        [[nodiscard]] bool isAbsoluteTopLimitReached() const;
        void initializeMachineValues(void);
        void initializeMotorsBundles(void);

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Shared<HAL::Sensors::ISensor> m_notBendingPosVirtualSensor;

        z_motors_bundle_t m_zMotorsBundle;
        stage_motor_bundle_t m_xMotorBundle;
        stage_motor_bundle_t m_yMotorBundle;
        stage_motor_bundle_t m_thetaMotorBundle;
        Shared<HAL::Act::IValve> m_waferVacuumValve;

        Config::process_config_t m_conf;
    };

}