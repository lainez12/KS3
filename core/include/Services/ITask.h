#pragma once

namespace Kub3::Services
{

    class ITask
    {
    public:
        virtual ~ITask() = default;

        // Called once when the task is started (reaches the front of the queue)
        virtual void start() = 0;

        // Must be called periodically. MUST return TRUE when the task is complete.
        virtual bool tick() = 0;
    };

} // namespace Kub3::Services
