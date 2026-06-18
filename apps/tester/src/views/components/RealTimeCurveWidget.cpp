#include <views/components/RealTimeCurveWidget.h>

#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <algorithm>

namespace Kub3::Tools::Tester
{

    RealTimeCurveWidget::RealTimeCurveWidget(QWidget *parent) : QWidget(parent)
    {
        setStyleSheet("background-color: #ffffff;");
        setMinimumSize(250, 250);

        // Drives visual scrolling at ~30 FPS
        connect(&m_scrollTimer, &QTimer::timeout, this, [this]() { update(); });
        m_scrollTimer.start(33);
    }

    void RealTimeCurveWidget::configureCurve(const QString &curveId, const QString &title, const QColor &color)
    {
        CurveInfo info;
        info.title        = title;
        info.color        = color;
        m_curves[curveId] = info;
        update();
    }

    void RealTimeCurveWidget::setUnit(const QString &unit)
    {
        m_unit = unit;
        update();
    }

    void RealTimeCurveWidget::setHistory(int milliseconds)
    {
        m_historyMs = milliseconds > 0 ? milliseconds : 5000;
        update();
    }

    void RealTimeCurveWidget::addDataPoint(const QString &curveId, double value)
    {
        if (!m_curves.contains(curveId))
            return;

        m_curves[curveId].currentValue = value;
        m_curves[curveId].data.push_back({QDateTime::currentMSecsSinceEpoch(), value});
        update();
    }

    void RealTimeCurveWidget::clear()
    {
        for (auto &curve : m_curves)
        {
            curve.data.clear();
            curve.currentValue = 0.0;
        }
        m_minY = -1.0;
        m_maxY = 1.0;
        update();
    }

    void RealTimeCurveWidget::calculateBounds()
    {
        bool hasData = false;
        double min   = 0.0;
        double max   = 0.0;

        for (const auto &curve : m_curves)
        {
            if (curve.data.empty())
                continue;

            if (!hasData)
            {
                min     = curve.currentValue;
                max     = curve.currentValue;
                hasData = true;
            }

            for (const auto &pt : curve.data)
            {
                if (pt.value < min)
                    min = pt.value;
                if (pt.value > max)
                    max = pt.value;
            }
        }

        if (!hasData)
            return;

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
        painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine));
        QFontMetrics fm = painter.fontMetrics();

        // --- Y-Axis ---
        for (int i = 0; i < 5; ++i)
        {
            double fraction = static_cast<double>(i) / 4.0;
            int y           = plotArea.bottom() - static_cast<int>(fraction * plotArea.height());
            double val      = m_minY + fraction * (m_maxY - m_minY);

            painter.drawLine(plotArea.left(), y, plotArea.right(), y);
            QString text = QString::number(val, 'f', 2);
            painter.setPen(Qt::black);
            painter.drawText(plotArea.left() - fm.horizontalAdvance(text) - 8, y + fm.height() / 4, text);
            painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine));
        }

        // --- X-Axis ---
        double totalSeconds = m_historyMs / 1000.0;
        for (int i = 0; i < 5; ++i)
        {
            double fraction = static_cast<double>(i) / 4.0;
            int x           = plotArea.left() + static_cast<int>(fraction * plotArea.width());
            double timeVal  = -totalSeconds + (fraction * totalSeconds);

            painter.drawLine(x, plotArea.top(), x, plotArea.bottom());
            QString text = QString::number(timeVal, 'f', 1) + "s";
            painter.setPen(Qt::black);
            painter.drawText(x - fm.horizontalAdvance(text) / 2, plotArea.bottom() + fm.height() + 5, text);
            painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine));
        }

        painter.setPen(QPen(Qt::black, 2));
        painter.drawLine(plotArea.bottomLeft(), plotArea.bottomRight());
        painter.drawLine(plotArea.topLeft(), plotArea.bottomLeft());
    }

    void RealTimeCurveWidget::drawLegend(QPainter &painter, const QRect &square, int marginLeft)
    {
        QFont titleFont = painter.font();
        titleFont.setBold(true);
        titleFont.setPointSize(9);
        painter.setFont(titleFont);

        int xOffset = square.left() + marginLeft;
        int yOffset = square.top() + 20;

        for (auto it = m_curves.begin(); it != m_curves.end(); ++it)
        {
            painter.setPen(it.value().color);
            QString legendText = QString("%1: %2 %3")
                                     .arg(it.value().title)
                                     .arg(it.value().currentValue, 0, 'f', 2)
                                     .arg(m_unit);

            painter.drawText(xOffset, yOffset, legendText);
            xOffset += painter.fontMetrics().horizontalAdvance(legendText) + 20;
        }
    }

    void RealTimeCurveWidget::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int side = std::min(width(), height());
        QRect square((width() - side) / 2, (height() - side) / 2, side, side);

        painter.fillRect(square, Qt::white);

        int marginLeft = 60, marginRight = 20, marginTop = 40, marginBottom = 30;
        if (square.width() < marginLeft + marginRight || square.height() < marginTop + marginBottom)
            return;

        QRect plotArea = square.adjusted(marginLeft, marginTop, -marginRight, -marginBottom);

        qint64 now         = QDateTime::currentMSecsSinceEpoch();
        qint64 windowStart = now - m_historyMs;

        // Purge old data across all curves
        for (auto &curve : m_curves)
        {
            while (curve.data.size() > 1 && curve.data[1].timestampMs < windowStart)
            {
                curve.data.pop_front();
            }
        }

        calculateBounds();
        drawGridAndAxes(painter, plotArea);
        drawLegend(painter, square, marginLeft);

        painter.setClipRect(plotArea);

        double range = m_maxY - m_minY;
        for (const auto &curve : m_curves)
        {
            QPainterPath path;
            if (curve.data.empty())
            {
                double y = plotArea.bottom() - ((curve.currentValue - m_minY) / range) * plotArea.height();
                path.moveTo(plotArea.left(), y);
                path.lineTo(plotArea.right(), y);
            }
            else
            {
                bool first = true;
                for (const auto &pt : curve.data)
                {
                    double x = plotArea.right() - ((now - pt.timestampMs) / static_cast<double>(m_historyMs)) * plotArea.width();
                    double y = plotArea.bottom() - ((pt.value - m_minY) / range) * plotArea.height();

                    if (first)
                    {
                        path.moveTo(x, y);
                        first = false;
                    }
                    else
                    {
                        path.lineTo(x, y);
                    }
                }
                double lastY = plotArea.bottom() - ((curve.data.back().value - m_minY) / range) * plotArea.height();
                path.lineTo(plotArea.right(), lastY);
            }

            painter.setPen(QPen(curve.color, 2));
            painter.drawPath(path);
        }
    }

} // namespace Kub3::Tools::Tester
