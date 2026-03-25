#pragma once

#include <string>

namespace Kub3::Services
{

    enum class ServiceStatus
    {
        Idle,
        Running,
        Success,
        Error
    };

    class IService
    {
    public:
        virtual ~IService() = default;

        virtual void tick(void) = 0;
        virtual void stop(void) = 0;

        [[nodiscard]] virtual ServiceStatus getStatus(void) const noexcept = 0;
        [[nodiscard]] virtual std::string getErrorReason(void) const       = 0;
    };

} // namespace Kub3::Services
