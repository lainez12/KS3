#pragma once

#include <variant>

#include <Services/IService.h>

namespace Kub3::Services
{

    struct BasicContactPayload;
    struct AutolevelingPayload;
    struct HorizontalityPayload;

    using ContactPayload = std::variant<BasicContactPayload, AutolevelingPayload, HorizontalityPayload>;

    typedef struct plan_relative_motors_positions_deltas_s {
        double left;
        double right;
        double back;
    } plan_deltas_t;

    enum class ZDirection
    {
        Up   = 0x0,
        Down = 0x1
    };

    struct ZMovePayload {
        ZDirection direction;
    };

    struct ZStopPayload {};

    using ZAxisPayload = std::variant<ZMovePayload, ZStopPayload>;

    class IContactService : public IService
    {
    public:
        virtual ~IContactService() = default;

        virtual void toggleForceSensors(bool en)              = 0;
        virtual void startContactRoutine(ContactPayload kind) = 0;
        virtual void moveZManual(ZDirection dir)              = 0;
        virtual void stopZManual(void)                        = 0;
        [[nodiscard]] virtual bool isInContact(void) const    = 0;
    };

}
