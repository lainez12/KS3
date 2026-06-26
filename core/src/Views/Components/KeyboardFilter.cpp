#include <Views/Components/KeyboardFilter.h>

#include <QApplication>
#include <QCoreApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QWidget>

namespace Kub3::UI::Views
{
    KeyboardFilter::KeyboardFilter(QWidget *scopeWidget, QObject *parent) : QObject(parent), m_scopeWidget(scopeWidget)
    {
        // Hook into the global application event dispatcher to intercept all events.
        if (qApp)
        {
            qApp->installEventFilter(this);
        }
    }

    KeyboardFilter::~KeyboardFilter()
    {
        if (qApp)
        {
            qApp->removeEventFilter(this);
        }
    }

    void KeyboardFilter::setHoldThreshold(uint32_t ms)
    {
        m_holdThresholdMs = ms;
    }

    bool KeyboardFilter::eventFilter(QObject *watched, QEvent *event)
    {
        if (!m_scopeWidget)
        {
            return QObject::eventFilter(watched, event);
        }

        if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
        {
            QWidget *targetWidget = qobject_cast<QWidget *>(watched);

            if (targetWidget)
            {
                // Prevent duplicate event processing by ensuring the event is for the active focused widget
                if (targetWidget != QApplication::focusWidget())
                {
                    return QObject::eventFilter(watched, event);
                }

                // Verify the focused widget belongs to our defined scope
                if (targetWidget == m_scopeWidget || m_scopeWidget->isAncestorOf(targetWidget))
                {
                    QKeyEvent *keyEvent        = static_cast<QKeyEvent *>(event);
                    Qt::Key key                = static_cast<Qt::Key>(keyEvent->key());
                    Qt::KeyboardModifiers mods = keyEvent->modifiers();

                    if (event->type() == QEvent::KeyPress)
                    {
                        // Ignore synthetic OS auto-repeat noise
                        if (!keyEvent->isAutoRepeat())
                        {
                            emit keyPressed(key, mods);

                            // Start a hold timer for this specific key if not already tracking it
                            if (!m_holdTimers.contains(key))
                            {
                                QTimer *timer = new QTimer(this);
                                timer->setSingleShot(true);
                                timer->setInterval(m_holdThresholdMs);

                                connect(timer, &QTimer::timeout, this, [this, key, mods]() {
                                    emit keyHeld(key, mods);
                                });

                                timer->start();
                                m_holdTimers.insert(key, timer);
                            }
                        }
                    }
                    else // QEvent::KeyRelease
                    {
                        if (!keyEvent->isAutoRepeat())
                        {
                            // Clean up the timer when the user physically releases the key
                            if (m_holdTimers.contains(key))
                            {
                                QTimer *timer = m_holdTimers.take(key);
                                timer->stop();
                                timer->deleteLater();
                            }

                            emit keyReleased(key, mods);
                        }
                    }

                    // Return false to let the target child process the character natively
                    return false;
                }
            }
        }

        return QObject::eventFilter(watched, event);
    }
} // namespace Kub3::UI::Views
