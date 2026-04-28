#pragma once

#include <Services/IService.h>
#include <variant>

namespace Kub3::Services
{
    enum class VisionMotor;
    enum class VisionDirection;

    // --- Pad Operation Payloads ---
    struct VisionMovePayload {
        VisionDirection dir;
    };
    struct VisionStopPayload {};
    struct VisionSetKinematicModePayload {
        bool fineMode;
    };
    struct VisionSetPushingModePayload {
        bool enable;
    };

    using VisionPayload = std::variant<
        VisionMovePayload,
        VisionStopPayload,
        VisionSetKinematicModePayload,
        VisionSetPushingModePayload>;

    class IVisionService : public IService
    {
    public:
        virtual ~IVisionService() = default;

        virtual void moveManual(VisionMotor motor, VisionDirection dir) = 0;
        virtual void stopManual(VisionMotor motor)                      = 0;
        virtual void setKinematicMode(VisionMotor motor, bool fineMode) = 0;
        virtual void setPushingMode(bool enabled)                       = 0;
    };
}
