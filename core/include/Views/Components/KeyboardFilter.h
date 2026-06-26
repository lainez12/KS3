#pragma once

#include <QHash>
#include <QObject>

class QKeyEvent;
class QWidget;
class QTimer;

namespace Kub3::UI::Views
{
    /**
     * @brief A global event filter designed to capture and process keyboard events.
     *
     * This filter intercepts key presses, holds, and releases for a specific scope widget
     * (and its children) while ignoring auto-repeat OS noise. It is robust enough to handle
     * multi-key combinations via independent timers.
     */
    class KeyboardFilter final : public QObject
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructs the KeyboardFilter and attaches it to the global application event loop.
         * @param scopeWidget The parent widget to monitor. Key events are only processed if focus is within this scope.
         * @param parent The QObject parent for memory management.
         */
        explicit KeyboardFilter(QWidget *scopeWidget, QObject *parent = nullptr);

        /**
         * @brief Destructor. Safely detaches the filter from the global application loop.
         */
        ~KeyboardFilter() override;

        /**
         * @brief Configures the duration a key must be held to trigger the `keyHeld` signal.
         * @param ms Time in milliseconds (default is 400ms).
         */
        void setHoldThreshold(uint32_t ms);

    signals:
        /**
         * @brief Emitted once when a physical key is initially pressed.
         */
        void keyPressed(Qt::Key key, Qt::KeyboardModifiers modifiers);

        /**
         * @brief Emitted once when a key has been held down longer than the threshold.
         */
        void keyHeld(Qt::Key key, Qt::KeyboardModifiers modifiers);

        /**
         * @brief Emitted once when a physical key is finally released.
         */
        void keyReleased(Qt::Key key, Qt::KeyboardModifiers modifiers);

    protected:
        /**
         * @brief Intercepts events before they reach their target.
         * @param watched The object receiving the event.
         * @param event The event being dispatched.
         * @return True if the event should be swallowed, false to allow normal propagation.
         */
        bool eventFilter(QObject *watched, QEvent *event) override;

    private:
        QWidget *m_scopeWidget = nullptr;

        uint32_t m_holdThresholdMs = 400;  ///< Time in ms before a press becomes a hold
        QHash<int, QTimer *> m_holdTimers; ///< Tracks independent hold timers for concurrent multi-key presses
    };
} // namespace Kub3::UI::Views
