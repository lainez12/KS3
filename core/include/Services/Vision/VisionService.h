#pragma once

#include <unordered_map>

#include <Config/conf.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/Vision/IVisionService.h>

namespace Kub3::Services
{
    enum class VisionMotor
    {
        UpperLeftCameraX,
        UpperLeftCameraY,
        UpperRightCameraX,
        UpperRightCameraY
    };

    enum class VisionDirection
    {
        Positive            = 0x0,
        Negative            = 0x1,
        UpperLeftCamXLeft   = Negative,
        UpperLeftCamXRight  = Positive,
        UpperLeftCamYBack   = Negative,
        UpperLeftCamYFront  = Positive,
        UpperRightCamXLeft  = Negative,
        UpperRightCamXRight = Positive,
        UpperRightCamYBack  = Negative,
        UpperRightCamYFront = Positive,
        DeckBack            = Negative,
        DeckFront           = Positive
    };

    constexpr uint32_t VISION_WATCHDOG_TIMEOUT_TICKS = 15; // 300ms

    struct vision_motor_config_t {
        Shared<HAL::Act::IPositionMotor> motor;
        Config::kinematic_profile_t fastProfile;
        Config::kinematic_profile_t fineProfile;
        double granularMovementMm;
        bool fineMode              = false;
        uint32_t watchdogTicks     = 0;
        VisionDirection currentDir = VisionDirection::Positive;
    };

    class VisionService final : public IVisionService
    {
    public:
        VisionService(Shared<HAL::Act::ActuatorRegistry> registry,
                      Shared<HAL::MS::IMachineStatusRepo> repo,
                      const Config::process_config_t &processConfig);

        void setLogCallback(LogCallback cb) override { m_logCallback = std::move(cb); };
        void tick(void) override;
        void stop(void) override;

        [[nodiscard]] ServiceStatus getStatus(void) const noexcept override
        {
            return m_status;
        }

        [[nodiscard]] std::string getErrorReason(void) const override
        {
            return m_errorReason;
        }

        // IVisionMotor overrides
        void moveBlockToVisualisationPosition(void) override;
        void moveManual(VisionMotor motor, VisionDirection dir, bool granular = false) override;
        void moveAbsolute(VisionMotor motor, double positionMm) override;
        void stopManual(VisionMotor motor) override;
        void setKinematicMode(VisionMotor motor, bool fineMode) override;
        void setPushingMode(bool enabled) override { m_pushingModeEnabled = enabled; }
        void setFocalEnabled(const std::string &focalId, bool enabled) override;
        void setFocalValue(const std::string &focalId, uint16_t val) override;

    private:
        // Collision and pushing logic
        void applyPush(VisionMotor pushingMotor, bool fineMode, bool granular = false);
        [[nodiscard]] bool inCollisionZone(VisionMotor motor, VisionDirection dir, bool granular = false) const;
        [[nodiscard]] bool isMotorBeingPushed(VisionMotor motor, VisionDirection lastDir) const;
        [[nodiscard]] std::optional<std::pair<VisionMotor, VisionDirection>> getAssociatedPushedMotor(VisionMotor motor, VisionDirection lastDir) const;
        void applyAntiCoastingStop(VisionMotor pushedMotor, VisionDirection pushedDir);

        // Misc.
        bool checkVirtualLimits(VisionMotor motorId, const vision_motor_config_t &config);
        void setupCameraMotor(VisionMotor motorId, const char *motorConfId, const Config::process_config_t &conf);
        bool deckVisualisationLimitReached(void) const;

    private:
        LogCallback m_logCallback;
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Config::vision_process_config_t m_conf;

        bool m_pushingModeEnabled = true;
        std::unordered_map<VisionMotor, vision_motor_config_t> m_cameraMotors;
        std::unordered_map<std::string, Config::focal_conf_t> m_focalConfs;

        // --- Deck Movement State ---
        Shared<HAL::Act::IMotor> m_deckMotor;
        Config::kinematic_profile_t m_deckProfile;
        const double m_deckVisuPosMm = 0.0;

        bool m_isDeckMoving    = false;
        ServiceStatus m_status = ServiceStatus::Idle;
        std::string m_errorReason;
    };
}
