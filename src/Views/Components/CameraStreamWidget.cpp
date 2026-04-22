#include "CameraStreamWidget.h"
#include <QPainter>
#include <QRect>

CameraStreamWidget::CameraStreamWidget(QWidget *parent) : QWidget(parent)
{
    // Optimization: Prevent background clearing before paintEvent
    setAttribute(Qt::WA_OpaquePaintEvent);
    // Ensure widget can expand in layouts
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void CameraStreamWidget::ps_onFrameUpdated(const QImage &frame)
{
    // Safe to copy implicitly due to Qt's Copy-On-Write. Our HAL deep copies earlier to ensure thread safety.
    m_currentFrame = frame;
    m_hasFrame     = true;

    update(); // Schedule a repaint in the UI thread event loop
}

void CameraStreamWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (!m_hasFrame || m_currentFrame.isNull())
    {
        // Draw placeholder/black screen
        painter.fillRect(event->rect(), Qt::black);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "NO SIGNAL");
        return;
    }

    // Calculate scaled rect while keeping aspect ratio
    QRect drawingRect = m_currentFrame.rect();
    drawingRect.moveCenter(rect().center());

    // Scale to fit widget size
    QImage scaledFrame = m_currentFrame.scaled(
        rect().size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation // Either Fast or Smooth
    );

    // Center the scaled image
    int x = (width() - scaledFrame.width()) / 2;
    int y = (height() - scaledFrame.height()) / 2;

    painter.fillRect(event->rect(), Qt::black); // Letterboxing borders
    painter.drawImage(x, y, scaledFrame);
}
