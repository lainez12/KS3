#pragma once

#include <functional>
#include <string>

#include <QDebug>

#include <Common/Enums.h>

namespace Kub3::Services
{

    template <typename TInterface>
    class BaseTaskService;

    class ITask
    {
        template <typename TInterface>
        friend class BaseTaskService;

    public:
        using LogCallback   = std::function<void(LogLevel, const std::string &)>;
        using AbortCallback = std::function<void(const std::string &)>;

        virtual ~ITask() = default;
        // Called once when the task is started (reaches the front of the queue)
        virtual void start() = 0;
        // Must be called periodically. MUST return TRUE when the task is complete.
        virtual bool tick() = 0;

        /**
         * @brief Indicates whether this task is a non-ending background task.
         *
         * Background tasks do not prevent the service sequence from completing.
         * The sequence will finish as soon as all foreground (non-background) tasks
         * across all lanes are completed.
         */
        [[nodiscard]] virtual bool isBackground() const noexcept { return false; }

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

        void abort(const std::string &reason)
        {
            if (m_abortCallback)
                m_abortCallback(reason);
            else
                qCritical() << "Task attempted to abort but failed to retrieve callback. Abort reason:" << reason;
        };

    private:
        // Setter for callbacks injection
        void setLogCallback(LogCallback cb) { m_logCallback = std::move(cb); }
        void setAbortCallback(AbortCallback cb) { m_abortCallback = std::move(cb); };

    private:
        LogCallback m_logCallback;
        AbortCallback m_abortCallback;
    };

} // namespace Kub3::Services
