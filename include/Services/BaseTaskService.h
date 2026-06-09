#pragma once

#include <QElapsedTimer>
#include <memory>
#include <queue>
#include <utility>

#include "IService.h"
#include "ITask.h"
#include <utils.h>

/**
 * @brief Default timeout for a sequence in the BaseTaskService (in milliseconds).
 */
#define BASE_TASK_SERVICE_TIMEOUT_MS 30000

namespace Kub3::Services
{

    /**
     * @brief A generic base class providing multi-lane task management for services.
     *
     * This class handles the queuing, ticking, and lifecycle management of `ITask` objects.
     * It supports concurrent "lanes" of execution, allowing independent sequences of tasks
     * to run in parallel. It also provides built-in watchdog timeout management.
     *
     * @tparam TInterface The specific service interface this class will implement.
     *                    Must inherit from `Kub3::Services::IService`.
     */
    template <typename TInterface>
    class BaseTaskService : public TInterface
    {
        static_assert(std::is_base_of_v<IService, TInterface>, "TInterface must inherit from Kub3::Services::IService");

    public:
        /**
         * @brief Default virtual destructor.
         */
        ~BaseTaskService() override = default;

        /**
         * @brief The universal tick function. Pumps the task queue and manages timeouts.
         *
         * This function should be called repeatedly by the main service runner.
         * It advances the active task in every execution lane, handles removing completed
         * tasks, starting the next tasks, and transitioning the service state to
         * Success or Error when finished or timed out.
         *
         * @todo Consider if this should return a boolean indicating completion.
         */
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

            bool allLanesEmpty = true;

            // Pump the active task in every lane
            for (auto &lane : m_lanes)
            {
                if (lane.empty())
                    continue;

                allLanesEmpty = false;

                // Tick the task at the front of this specific lane
                bool isCurrentTaskDone = lane.front()->tick();

                if (isCurrentTaskDone)
                {
                    lane.pop(); // Remove finished task

                    if (!lane.empty())
                        lane.front()->start(); // Start the next task in this lane
                }
            }

            // If all queues in all lanes are empty, the entire service is done
            if (allLanesEmpty)
            {
                m_status = ServiceStatus::Success;
            }
        }

        /**
         * @brief Universally stops the service execution.
         *
         * Clears all pending and running tasks across all lanes and sets the
         * service status back to Idle.
         */
        void stop(void) override
        {
            this->onStop();
            clearTasks();
            m_status = ServiceStatus::Idle;
        }

        /**
         * @brief Retrieves the current status of the service.
         *
         * @return ServiceStatus The current status (Idle, Running, Success, or Error).
         */
        [[nodiscard]] inline ServiceStatus getStatus() const noexcept override
        {
            return m_status;
        }

        /**
         * @brief Retrieves the reason for a sequence failure.
         *
         * @return std::string A human-readable error description, empty if no error occurred.
         */
        [[nodiscard]] inline std::string getErrorReason() const override
        {
            return m_errorReason;
        }

    protected:
        // Protected API for inheriting services (Drawer, Alignment, etc...) to use

        /**
         * @brief Enqueues a pre-constructed task into a specific execution lane.
         *
         * If the requested lane does not exist, the internal lane array is dynamically
         * resized to accommodate it.
         *
         * @param task A unique pointer to the `ITask` to be enqueued.
         * @param lane The zero-based index of the execution lane. Defaults to 0.
         */
        void enqueueTask(Unique<ITask> task, uint8_t lane = 0)
        {
            // Dynamically allocate new lanes if a higher lane index is requested
            if (lane >= m_lanes.size())
            {
                m_lanes.resize(lane + 1);
            }
            m_lanes[lane].push(std::move(task));
        }

        /**
         * @brief Constructs and enqueues a task into a specific lane in-place.
         *
         * @tparam _Tp The type of the task to instantiate.
         * @tparam Lane The zero-based index of the execution lane (compile-time parameter). Defaults to 0.
         * @tparam _Args Variadic template types for the constructor arguments.
         * @param __args Arguments forwarded to the constructor of `_Tp`.
         */
        template <typename _Tp, uint8_t Lane = 0, typename... _Args>
        void enqueueTask(_Args &&...__args)
        {
            auto task = std::make_unique<_Tp>(std::forward<_Args>(__args)...);

            enqueueTask(std::move(task), Lane);
        }

        /**
         * @brief Initializes and starts the execution sequence.
         *
         * Prepares the watchdog timer, sets the state to Running, and triggers
         * the `start()` method of the first task in every populated lane.
         *
         * @param timeoutMs Maximum allowed execution time in milliseconds before triggering an abort.
         *                  Defaults to BASE_TASK_SERVICE_TIMEOUT_MS.
         */
        void startSequence(int32_t timeoutMs = BASE_TASK_SERVICE_TIMEOUT_MS)
        {
            m_timeoutMs = timeoutMs;
            m_status    = ServiceStatus::Running;
            m_errorReason.clear();
            m_watchdog.start();

            bool hasTasks = false;

            // Start the front task of every active lane
            for (auto &lane : m_lanes)
            {
                if (!lane.empty())
                {
                    lane.front()->start();
                    hasTasks = true;
                }
            }

            if (!hasTasks) // Started with all queues empty
                m_status = ServiceStatus::Success;
        }

        /**
         * @brief Optional hardware safety shutdown hook for derived classes.
         * Override this to stop motors, turn off valves, etc.
         */
        virtual void onStop(void) {}

        /**
         * @brief Aborts the current execution sequence.
         *
         * Clears all tasks in all lanes, updates the service status to Error,
         * and records the given failure reason.
         *
         * @note Specific derived services should override `stop()` if they need to send
         *       hardware emergency stops to motors before calling `BaseTaskService::stop()`.
         *
         * @param reason Human-readable string explaining why the sequence was aborted.
         */
        virtual void abortSequence(const std::string &reason)
        {
            this->onStop();
            clearTasks();
            m_status      = ServiceStatus::Error;
            m_errorReason = reason;
        }

        /**
         * @brief Clears all queued and active tasks across all lanes.
         */
        void clearTasks(void)
        {
            m_lanes.clear();
        }

    protected:
        ServiceStatus m_status = ServiceStatus::Idle; ///< Current operational status of the service
        std::string m_errorReason;                    ///< Error message if the sequence failed or aborted

    private:
        std::deque<std::queue<Unique<ITask>>> m_lanes; ///< Multi-lane structure holding tasks for parallel execution
        QElapsedTimer m_watchdog;                      ///< Timer tracking sequence execution duration
        int32_t m_timeoutMs = 0;                       ///< Allocated maximum sequence time (0 implies no timeout)
    };

} // namespace Kub3::Services