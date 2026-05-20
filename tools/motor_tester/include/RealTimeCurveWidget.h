#pragma once

#include <QColor>
#include <QString>
#include <QWidget>
#include <deque>

namespace Kub3::Tools::MotorTester
{

    class RealTimeCurveWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit RealTimeCurveWidget(QWidget *parent = nullptr);
        ~RealTimeCurveWidget() override = default;

        // Configuration
        void setCurveProperties(const QString &title, const QString &unit, const QColor &color);
        void setHistory(size_t maxPoints, double hz);

    public slots:
        void addDataPoint(double value);
        void clear();

    protected:
        void paintEvent(QPaintEvent *event) override;

    private:
        void calculateBounds();
        void drawGridAndAxes(QPainter &painter, const QRect &plotArea);

    private:
        QString m_title = "Value";
        QString m_unit  = "";
        QColor m_color  = Qt::cyan;

        size_t m_maxPoints = 250;
        double m_hz        = 50.0;
        std::deque<double> m_data;

        double m_currentValue = 0.0;
        double m_minY         = -1.0;
        double m_maxY         = 1.0;
    };

} // namespace Kub3::Tools::MotorTester
