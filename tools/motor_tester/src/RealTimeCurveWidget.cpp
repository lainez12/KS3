#include "RealTimeCurveWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <algorithm>
#include <cmath>

namespace Kub3::Tools::MotorTester
{

    RealTimeCurveWidget::RealTimeCurveWidget(QWidget *parent) : QWidget(parent)
    {
        // Enforce white background
        setStyleSheet("background-color: #dddddd;");
        setMinimumSize(250, 250); // Need space for the axes
    }

    void RealTimeCurveWidget::setCurveProperties(const QString &title, const QString &unit, const QColor &color)
    {
        m_title = title;
        m_unit  = unit;
        m_color = color;
        update();
    }

    void RealTimeCurveWidget::setHistory(size_t maxPoints, double hz)
    {
        m_maxPoints = maxPoints;
        m_hz        = hz > 0.0 ? hz : 50.0;
        update();
    }

    void RealTimeCurveWidget::addDataPoint(double value)
    {
        m_currentValue = value;
        m_data.push_back(value);

        if (m_data.size() > m_maxPoints)
        {
            m_data.pop_front();
        }

        update();
    }

    void RealTimeCurveWidget::clear()
    {
        m_data.clear();
        m_currentValue = 0.0;
        m_minY         = -1.0;
        m_maxY         = 1.0;
        update();
    }

    void RealTimeCurveWidget::calculateBounds()
    {
        if (m_data.empty())
            return;

        double min = m_data.front();
        double max = m_data.front();

        for (double v : m_data)
        {
            if (v < min)
                min = v;
            if (v > max)
                max = v;
        }

        // Padding and division-by-zero protection
        double range = max - min;
        if (range < 0.001)
        {
            range = 2.0;
            min -= 1.0;
            max += 1.0;
        }

        m_minY = min - (range * 0.1);
        m_maxY = max + (range * 0.1);
    }

    void RealTimeCurveWidget::drawGridAndAxes(QPainter &painter, const QRect &plotArea)
    {
        painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine)); // Light gray grid
        QFontMetrics fm = painter.fontMetrics();

        // --- Y-Axis (Ordinate) ---
        int yTicks = 5;
        for (int i = 0; i < yTicks; ++i)
        {
            double fraction = static_cast<double>(i) / (yTicks - 1);
            int y           = plotArea.bottom() - static_cast<int>(fraction * plotArea.height());
            double val      = m_minY + fraction * (m_maxY - m_minY);

            // Draw horizontal grid line
            painter.drawLine(plotArea.left(), y, plotArea.right(), y);

            // Draw Y-axis text
            QString text = QString::number(val, 'f', 2);
            painter.setPen(Qt::black);
            painter.drawText(plotArea.left() - fm.horizontalAdvance(text) - 8,
                             y + fm.height() / 4, text);
            painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine)); // restore pen
        }

        // --- X-Axis (Abscissa) ---
        int xTicks          = 5;
        double totalSeconds = m_maxPoints / m_hz;
        for (int i = 0; i < xTicks; ++i)
        {
            double fraction = static_cast<double>(i) / (xTicks - 1);
            int x           = plotArea.left() + static_cast<int>(fraction * plotArea.width());

            // X value goes from -totalSeconds to 0
            double timeVal = -totalSeconds + (fraction * totalSeconds);

            // Draw vertical grid line
            painter.drawLine(x, plotArea.top(), x, plotArea.bottom());

            // Draw X-axis text
            QString text = QString::number(timeVal, 'f', 1) + "s";
            painter.setPen(Qt::black);
            painter.drawText(x - fm.horizontalAdvance(text) / 2,
                             plotArea.bottom() + fm.height() + 5, text);
            painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine)); // restore pen
        }

        // Draw solid axes lines
        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(plotArea.bottomLeft(), plotArea.bottomRight()); // X axis
        painter.drawLine(plotArea.topLeft(), plotArea.bottomLeft());     // Y axis
    }

    void RealTimeCurveWidget::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Force Square Aspect Ratio inside the widget
        int side = std::min(width(), height());
        QRect square((width() - side) / 2, (height() - side) / 2, side, side);

        painter.fillRect(square, Qt::white);

        // Define Plot Area (Margins for axes and text)
        int marginLeft   = 60; // Space for Y-axis numbers
        int marginRight  = 20;
        int marginTop    = 40; // Space for Title
        int marginBottom = 30; // Space for X-axis numbers

        // Prevent drawing if the widget is too small
        if (square.width() < marginLeft + marginRight || square.height() < marginTop + marginBottom)
        {
            return;
        }

        QRect plotArea = square.adjusted(marginLeft, marginTop, -marginRight, -marginBottom);

        // Update Bounds and Draw Axes
        calculateBounds();
        drawGridAndAxes(painter, plotArea);

        // Draw Title and Current Value
        painter.setPen(Qt::black);
        QFont titleFont = painter.font();
        titleFont.setBold(true);
        titleFont.setPointSize(10);
        painter.setFont(titleFont);

        QString headerText = QString("%1: %2 %3")
                                 .arg(m_title)
                                 .arg(m_currentValue, 0, 'f', 2)
                                 .arg(m_unit);
        painter.drawText(square.left() + marginLeft, square.top() + 25, headerText);

        // Draw the Curve Data
        if (m_data.empty())
            return;

        // Clip drawing to the plot area so lines don't bleed into the axes
        painter.setClipRect(plotArea);

        QPainterPath path;
        double range = m_maxY - m_minY;
        double xStep = static_cast<double>(plotArea.width()) / static_cast<double>(m_maxPoints - 1);

        // Optimization: Start drawing from the correct offset if history isn't full yet
        int startOffset = m_maxPoints - m_data.size();

        for (size_t i = 0; i < m_data.size(); ++i)
        {
            double x           = plotArea.left() + ((startOffset + i) * xStep);
            double normalizedY = (m_data[i] - m_minY) / range;
            double y           = plotArea.bottom() - (normalizedY * plotArea.height());

            if (i == 0)
            {
                path.moveTo(x, y);
            }
            else
            {
                path.lineTo(x, y);
            }
        }

        painter.setPen(QPen(m_color, 2));
        painter.drawPath(path);
    }

} // namespace Kub3::Tools::MotorTester
