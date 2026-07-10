#pragma once

#include <Common/Enums.h>
#include <HAL/Actuators/Motors/IMotor.h>
#include <Services/IService.h>

namespace Kub3::Services
{

    enum class AlignmentDirection
    {
        POSITIVE          = static_cast<int>(HAL::Act::MotorDirection::Positive),
        NEGATIVE          = static_cast<int>(HAL::Act::MotorDirection::Negative),
        LEFT              = POSITIVE,
        RIGHT             = NEGATIVE,
        BACK              = POSITIVE,
        FRONT             = NEGATIVE,
        COUNTER_CLOCKWISE = POSITIVE,
        CLOCKWISE         = NEGATIVE,
    };

    struct AlignmentMoveStagePayload {
        AlignmentDirection dir;
        bool granular = true;
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

        virtual void moveStage(AlignmentStageId axis, AlignmentDirection dir, bool granular = false) = 0;
        virtual void stopStage(AlignmentStageId axis)                                                = 0;
        virtual void setKinematicProfile(AlignmentStageId axis, bool fineMode)                       = 0;
        virtual void setHardwareLock(bool locked)                                                    = 0;
    };

}
