#pragma once

#include <Config/conf.h>
#include <Config/kinematics.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <Services/Alignment/IAlignmentService.h>
#include <Services/BaseTaskService.h>
#include <utils.h>

namespace Kub3::Services
{
    constexpr uint32_t ALIGNMENT_WATCHDOG_TIMEOUT_TICKS = 15u;

    typedef struct motor_alignment_config_s {
        Shared<HAL::Act::IPositionMotor> motor;
        Config::kinematic_profile_t fastProfile;
        Config::kinematic_profile_t fineProfile;
        double granularMoveMm;
        bool fineMode         = true;
        uint8_t watchdogTicks = 0;
    } motor_alignment_config_t;

    class AlignmentService final : public IAlignmentService
    {
    public:
        AlignmentService(Shared<HAL::Act::ActuatorRegistry> registry,
                         Shared<HAL::MS::IMachineStatusRepo> repo,
                         const Config::process_config_t &processConfig);

        void setLogCallback(LogCallback cb) override { m_logCallback = std::move(cb); };
        void tick() override;
        void stop() override;

        [[nodiscard]] inline ServiceStatus getStatus(void) const noexcept override
        {
            return ServiceStatus::Running;
        };

        [[nodiscard]] inline std::string getErrorReason(void) const override
        {
            return "";
        };

        void moveStage(AlignmentStageId axis, AlignmentDirection dir, bool granular = false) override;
        void stopStage(AlignmentStageId axis) override;
        void setKinematicProfile(AlignmentStageId axis, bool fineMode) override;
        void setHardwareLock(bool locked) override;

    private:
        void loadConfigurations(Shared<HAL::Act::ActuatorRegistry> registry, const Config::process_config_t &processConfig);

    private:
        LogCallback m_logCallback;
        bool m_isLocked = false;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        std::unordered_map<AlignmentStageId, motor_alignment_config_t> m_motorsConfigurations;

        // Fast kinematic profiles
        Config::kinematic_profile_t m_xStageFastProfile;
        Config::kinematic_profile_t m_yStageFastProfile;
        Config::kinematic_profile_t m_thetaStageFastProfile;
        // Fine kinematic profiles
        Config::kinematic_profile_t m_xStageFineProfile;
        Config::kinematic_profile_t m_yStageFineProfile;
        Config::kinematic_profile_t m_thetaStageFineProfile;
    };

}
