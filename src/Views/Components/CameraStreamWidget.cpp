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

    // This creates the "letterbox" borders if the layout stretches the widget to a rectangle.
    painter.fillRect(event->rect(), Qt::black);

    if (!m_hasFrame || m_currentFrame.isNull())
    {
        // Draw placeholder/black screen
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "NO SIGNAL");
        return;
    }

    // Calculate the largest perfect square that fits in the widget's current size
    int side    = std::min(width(), height());
    int xOffset = (width() - side) / 2;
    int yOffset = (height() - side) / 2;

    QImage scaledImage = m_currentFrame.scaled(
        side, side,
        Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation);

    // painter.setRenderHint(QPainter::LosslessImageRendering);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(xOffset, yOffset, scaledImage);
}
