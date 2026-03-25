#pragma once

#include <QElapsedTimer>
#include <memory>
#include <queue>
#include <utility>

#include "IService.h"
#include "ITask.h"
#include <utils.h>

#define BASE_TASK_SERVICE_TIMEOUT_MS 30000

namespace Kub3::Services
{

    template <typename TInterface>
    class BaseTaskService : public TInterface
    {
        static_assert(std::is_base_of_v<IService, TInterface>, "TInterface must inherit from Kub3::Services::IService");

    public:
        ~BaseTaskService() override = default;

        // TODO: should return true once finished ?
        // The universal tick function. It pumps the queue and manages timeouts.
        void tick(void) override
        {
            if (m_status != ServiceStatus::Running)
                return;

            // Timeout Watchdog
            if (m_timeoutMs > 0 && m_watchdog.hasExpired(m_timeoutMs))
            {
                abortSequence("Service sequence timed out.");
                return;
            }

            // Queue Evaluation
            if (m_taskQueue.empty())
            {
                m_status = ServiceStatus::Success; // All tasks finished
                return;
            }

            // Pump the active task
            bool isCurrentTaskDone = m_taskQueue.front()->tick();

            if (isCurrentTaskDone)
            {
                m_taskQueue.pop(); // Remove finished task

                if (!m_taskQueue.empty())
                    m_taskQueue.front()->start(); // Start the next task immediately if one exists
                else
                    m_status = ServiceStatus::Success;
            }
        }

        // Default universal stop
        void stop(void) override
        {
            clearTasks();
            m_status = ServiceStatus::Idle;
        }

        [[nodiscard]] inline ServiceStatus getStatus() const noexcept override
        {
            return m_status;
        }

        [[nodiscard]] inline std::string getErrorReason() const override
        {
            return m_errorReason;
        }

    protected:
        // Protected API for inheriting services (Drawer, Alignment, etc...) to use

        void enqueueTask(Unique<ITask> task)
        {
            m_taskQueue.push(std::move(task));
        }

        void startSequence(int32_t timeoutMs = BASE_TASK_SERVICE_TIMEOUT_MS)
        {
            m_timeoutMs = timeoutMs;
            m_status    = ServiceStatus::Running;
            m_errorReason.clear();
            m_watchdog.start();

            if (!m_taskQueue.empty())
                m_taskQueue.front()->start();
            else // Started with an empty queue
                m_status = ServiceStatus::Success;
        }

        void abortSequence(const std::string &reason)
        {
            clearTasks();
            m_status      = ServiceStatus::Error;
            m_errorReason = reason;

            // @note: Specific services should override stop() if they need to send
            // emergencyStop() to motors, then call BaseTaskService::stop().
        }

        void clearTasks(void)
        {
            std::queue<Unique<ITask>> empty;
            std::swap(m_taskQueue, empty);
        }

    protected:
        ServiceStatus m_status = ServiceStatus::Idle;
        std::string m_errorReason;

    private:
        std::queue<Unique<ITask>> m_taskQueue;
        QElapsedTimer m_watchdog;
        int32_t m_timeoutMs = 0;
    };

} // namespace Kub3::Services