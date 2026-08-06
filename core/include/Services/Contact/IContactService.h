#pragma once

#include <variant>

#include <Services/IService.h>

namespace Kub3
{
    enum class ForceSensor : uint32_t;
    class TestToken;
}

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

    struct ZMovePayload {
        ZDirection direction;
        bool granular = false;
    };

    struct ZStopPayload {};

    using ZAxisPayload = std::variant<ZMovePayload, ZStopPayload>;

    class IContactService : public IService
    {
    public:
        virtual ~IContactService() = default;

        virtual void startContactRoutine(ContactPayload kind)                 = 0;
        virtual void retractFromContact(void)                                 = 0;
        virtual void moveZManual(ZDirection dir, bool granular = false)       = 0;
        virtual void stopZManual(void)                                        = 0;
        virtual void processBackgroundAutomations(void)                       = 0;
        virtual void setSubstrateCompressedAir(bool enable)                   = 0;
        [[nodiscard]] virtual bool isInContact(void) const                    = 0;
        [[nodiscard]] virtual bool isSubstrateCompressedAirActive(void) const = 0;

        // Test methods
        virtual void toggleForceSensors(TestToken, bool en)     = 0;
        virtual void tareForceSensor(TestToken, ForceSensor fs) = 0;
    };

}
