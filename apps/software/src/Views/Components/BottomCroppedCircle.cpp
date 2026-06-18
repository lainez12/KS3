#include <QPainter>

#include <Views/Components/BottomCroppedCircle.h>

BottomCroppedCircle::BottomCroppedCircle(QWidget *parent) : QWidget(parent) {}

void BottomCroppedCircle::setColor(const QColor &color)
{
    m_color = color;
    update();
}

void BottomCroppedCircle::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int side = qMin(width(), height()); // Always draw a perfect circle based on the smallest available dimension
    int x    = (width() - side) / 2;
    int y    = (height() - side) / 2;
    QRect rect(x, y, side, side);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(0, 0, width(), y + (side * 0.96)); // CROP: Tell the painter to ONLY draw in the top 96% of the circle
    // Draw the shape
    painter.setBrush(m_color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(rect);
}
