#pragma once

#include <HAL/Actuators/Motors/IMotor.h>
#include <Services/IService.h>

namespace Kub3::Services
{

    enum class AlignmentStage
    {
        X     = 0x0,
        Y     = 0x1,
        THETA = 0x2
    };

    enum class AlignmentDirection
    {
        POSITIVE          = static_cast<int>(HAL::Act::MotorDirection::Positive),
        NEGATIVE          = static_cast<int>(HAL::Act::MotorDirection::Negative),
        LEFT              = POSITIVE,
        RIGHT             = NEGATIVE,
        BACK              = POSITIVE,
        FRONT             = NEGATIVE,
        CLOCKWISE         = POSITIVE,
        COUNTER_CLOCKWISE = NEGATIVE,
    };

    struct AlignmentMoveStagePayload {
        AlignmentDirection dir;
    };

    struct AlignmentStopStagePayload {};

    struct AlignmentSetKinematicModePayload {
        bool fineMode;
    };

    using AlignmentPayload = std::variant<
        AlignmentMoveStagePayload,
        AlignmentStopStagePayload,
        AlignmentSetKinematicModePayload>;

    class IAlignmentService : public IService
    {
    public:
        virtual ~IAlignmentService() = default;

        virtual void moveStage(AlignmentStage axis, AlignmentDirection dir)  = 0;
        virtual void stopStage(AlignmentStage axis)                          = 0;
        virtual void setKinematicProfile(AlignmentStage axis, bool fineMode) = 0;
        virtual void setHardwareLock(bool locked)                            = 0;
    };

}
