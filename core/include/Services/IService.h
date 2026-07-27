#pragma once

#include <string>

#include <Common/Enums.h>

#define UNWRAP_OR_THROW(var_name, result_expr, error_prefix)                                \
    auto _res_##var_name = (result_expr);                                                   \
    if (!_res_##var_name)                                                                   \
    {                                                                                       \
        qCritical().noquote() << error_prefix << _res_##var_name.unwrap_err().c_str();      \
        throw std::runtime_error(std::string(error_prefix) + _res_##var_name.unwrap_err()); \
    }                                                                                       \
    auto var_name = _res_##var_name.unwrap()

#define UNWRAP_OR_ABORT(var_name, result_expr)             \
    auto _res_##var_name = (result_expr);                  \
    if (!_res_##var_name)                                  \
    {                                                      \
        this->abortSequence(_res_##var_name.unwrap_err()); \
        return;                                            \
    }                                                      \
    const auto var_name = _res_##var_name.unwrap()

#define STOP_MOTOR_RESULT(motorId, registry)                                                             \
    do                                                                                                   \
    {                                                                                                    \
        if (auto m = registry->get<HAL::Act::IMotor>(motorId))                                           \
            m->emergencyStop();                                                                          \
        else                                                                                             \
            qCritical().noquote() << "[HomingService] Failed to stop motor with identifier:" << motorId; \
    } while (0);

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
        using LogCallback = std::function<void(LogLevel, const std::string &)>;

        virtual ~IService()                         = default;
        virtual void setLogCallback(LogCallback cb) = 0;

        virtual void tick(void) = 0;
        virtual void stop(void) = 0;

        [[nodiscard]] virtual ServiceStatus getStatus(void) const noexcept = 0;
        [[nodiscard]] virtual std::string getErrorReason(void) const       = 0;
    };

} // namespace Kub3::Services
