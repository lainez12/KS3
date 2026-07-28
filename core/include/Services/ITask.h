#pragma once

#include <functional>
#include <string>

#include <Common/Enums.h>

namespace Kub3::Services
{

    class ITask
    {
    public:
        using LogCallback = std::function<void(LogLevel, const std::string &)>;

        virtual ~ITask() = default;
        // Called once when the task is started (reaches the front of the queue)
        virtual void start() = 0;
        // Must be called periodically. MUST return TRUE when the task is complete.
        virtual bool tick() = 0;
        // Setter for the callback injection
        void setLogCallback(LogCallback cb) { m_logCallback = std::move(cb); }

    protected:
        // Protected helpers for derived Tasks (e.g. MoveMotorTask)
        void postLog(LogLevel level, const std::string &msg)
        {
            if (m_logCallback)
                m_logCallback(level, msg);
        }
        void postInfo(const std::string &msg) { postLog(LogLevel::Info, msg); }
        void postSuccess(const std::string &msg) { postLog(LogLevel::Success, msg); }
        void postWarning(const std::string &msg) { postLog(LogLevel::Warning, msg); }
        void postError(const std::string &msg) { postLog(LogLevel::Error, msg); }

    private:
        LogCallback m_logCallback;
    };

} // namespace Kub3::Services
