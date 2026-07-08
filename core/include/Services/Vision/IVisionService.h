#pragma once

#include <QString>
#include <variant>

#include <Services/IService.h>

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

        // Movement pure virtual methods
        virtual void moveBlockToVisualisationPosition(void)                                    = 0;
        virtual void moveManual(VisionMotor motor, VisionDirection dir, bool granular = false) = 0;
        virtual void stopManual(VisionMotor motor)                                             = 0;
        virtual void setKinematicMode(VisionMotor motor, bool fineMode)                        = 0;
        virtual void setPushingMode(bool enabled)                                              = 0;

        // Focals pure virtual methods
        virtual void setFocalEnabled(const std::string &focalId, bool enabled) = 0;
        virtual void setFocalValue(const std::string &focalId, uint16_t val)   = 0;
    };
}
