#pragma once

#include <QColor>
#include <QMap>
#include <QString>
#include <QTimer>
#include <QWidget>
#include <deque>

namespace Kub3::Tools::Tester
{

    struct TimePoint {
        qint64 timestampMs;
        double value;
    };

    // Helper structure to hold individual curve parameters and data
    struct CurveInfo {
        QString title;
        QColor color;
        std::deque<TimePoint> data;
        double currentValue = 0.0;
    };

    class RealTimeCurveWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit RealTimeCurveWidget(QWidget *parent = nullptr);
        ~RealTimeCurveWidget() override = default;

        // Replaces setCurveProperties: Registers a new curve on this plot
        void configureCurve(const QString &curveId, const QString &title, const QColor &color);

        // Configures the time window (e.g., 30000 for 30s)
        void setHistory(int milliseconds);

        // Configures the measurement unit displayed on the Y-Axis (e.g., "mm", "mm/s")
        void setUnit(const QString &unit);

    public slots:
        // Replaces the single-value overload to target a specific configured curve
        void addDataPoint(const QString &curveId, double value);
        void clear();

        // Locks or unlocks the Y-axis bounds (stops auto-scaling)
        void setScaleLocked(bool locked);

    protected:
        void paintEvent(QPaintEvent *event) override;

    private:
        void calculateBounds();
        void drawGridAndAxes(QPainter &painter, const QRect &plotArea);
        void drawLegend(QPainter &painter, const QRect &square, int marginLeft);

    private:
        QString m_unit  = "";
        int m_historyMs = 5000;

        double m_minY = -1.0;
        double m_maxY = 1.0;

        bool m_scaleLocked = false; // Tracks if the Y-axis bounds are locked

        // Collection of curves keyed by their ID (e.g., "ref_pos", "real_pos")
        QMap<QString, CurveInfo> m_curves;

        QTimer m_scrollTimer; // Drives the visual scrolling independent of data events
    };

} // namespace Kub3::Tools::Tester