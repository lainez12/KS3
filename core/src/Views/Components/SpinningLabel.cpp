#include <Views/Components/SpinningLabel.h>

#include <QResizeEvent>
#include <QTransform>

namespace Kub3::UI::Views
{
    SpinningLabel::SpinningLabel(QWidget *parent) :
        QLabel(parent)
    {
        // Smooth scaling & alignment defaults
        setScaledContents(false);
        setAlignment(Qt::AlignCenter);

        connect(&m_spinTimer, &QTimer::timeout, this, &SpinningLabel::onSpinTimerTimeout);
    }

    void SpinningLabel::setRawPixmap(const QPixmap &pixmap)
    {
        m_rawPixmap = pixmap;
        updateDisplay();
    }

    void SpinningLabel::startSpinning(int intervalMs, int stepAngle)
    {
        m_stepAngle  = stepAngle;
        m_isSpinning = true;

        if (!m_spinTimer.isActive())
        {
            m_spinTimer.start(intervalMs);
        }
    }

    void SpinningLabel::stopSpinning()
    {
        m_isSpinning   = false;
        m_currentAngle = 0;
        m_spinTimer.stop();
        updateDisplay();
    }

    void SpinningLabel::resizeEvent(QResizeEvent *event)
    {
        QLabel::resizeEvent(event);

        // The moment Qt lays out the widget and gives it a non-zero size,
        // updateDisplay() will scale and display the raw pixmap.
        updateDisplay();
    }

    void SpinningLabel::onSpinTimerTimeout()
    {
        if (!m_isSpinning)
            return;

        m_currentAngle = (m_currentAngle + m_stepAngle) % 360;
        updateDisplay();
    }

    void SpinningLabel::updateDisplay()
    {
        // Don't attempt to render if no pixmap exists or geometry is not calculated yet
        if (m_rawPixmap.isNull() || size().isEmpty())
        {
            return;
        }

        QPixmap outputPixmap = m_rawPixmap;

        // Apply rotation if spinning
        if (m_isSpinning && m_currentAngle != 0)
        {
            QTransform transform;
            transform.rotate(m_currentAngle);
            outputPixmap = outputPixmap.transformed(transform, Qt::SmoothTransformation);
        }

        // Scale smoothly to fit the label's actual layout bounds
        outputPixmap = outputPixmap.scaled(
            size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        // Call underlying QLabel::setPixmap
        QLabel::setPixmap(outputPixmap);
    }

} // namespace Kub3::UI::Components
