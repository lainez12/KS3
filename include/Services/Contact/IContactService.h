#pragma once

#include <variant>

#include <Services/IService.h>

namespace Kub3::Services
{

    struct BasicContactPayload;
    struct AutolevelingPayload;

    using ContactPayload = std::variant<
        BasicContactPayload,
        AutolevelingPayload>;

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

        virtual void startContactRoutine(ContactPayload kind) = 0;
        virtual void moveZManual(ZDirection dir)              = 0;
        virtual void stopZManual(void)                        = 0;
    };

}
