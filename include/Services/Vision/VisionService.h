#pragma once

#include "IVisionService.h"
#include <Config/conf.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <array>
#include <unordered_map>

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
        UpperRightCamXLeft  = Positive,
        UpperRightCamXRight = Negative,
        UpperRightCamYBack  = Negative,
        UpperRightCamYFront = Positive,
        DeckBack            = Negative,
        DeckFront           = Positive
    };

    constexpr uint32_t VISION_WATCHDOG_TIMEOUT_TICKS = 15; // 300ms

    struct vision_motor_config_t {
        Shared<HAL::Act::IMotor> motor;
        Config::kinematic_profile_t fastProfile;
        Config::kinematic_profile_t fineProfile;

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
        void moveManual(VisionMotor motor, VisionDirection dir) override;
        void stopManual(VisionMotor motor) override;
        void setKinematicMode(VisionMotor motor, bool fineMode) override;
        void setPushingMode(bool enabled) override
        {
            m_pushingModeEnabled = enabled;
        }

    private:
        [[nodiscard]] bool inCollisionZone(VisionMotor motor, VisionDirection dir) const;
        void setupMotor(VisionMotor motorId, const char *motorConfId, const Config::process_config_t &conf);
        void applyPush(VisionMotor pushingMotor, bool fineMode);
        bool deckVisualisationLimitReached(void) const;

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;

        bool m_pushingModeEnabled    = false;
        double m_minCameraDistanceMm = 0.0f; // Minimal safe distance to prevent collision
        std::unordered_map<VisionMotor, vision_motor_config_t> m_cameraMotors;

        // --- Deck Movement State ---
        Shared<HAL::Act::IMotor> m_deckMotor;
        Config::kinematic_profile_t m_deckProfile;
        const double m_deckVisuPosMm = 0.0;

        bool m_isDeckMoving    = false;
        ServiceStatus m_status = ServiceStatus::Idle;
        std::string m_errorReason;
    };
}
