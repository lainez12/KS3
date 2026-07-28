#pragma once

#include <QLabel>
#include <QPixmap>
#include <QTimer>

namespace Kub3::UI::Views
{
    class SpinningLabel : public QLabel
    {
        Q_OBJECT

    public:
        explicit SpinningLabel(QWidget *parent = nullptr);
        ~SpinningLabel() override = default;

        // Custom setPixmap to store the raw unscaled pixmap
        void setRawPixmap(const QPixmap &pixmap);

        // Animation control
        void startSpinning(int intervalMs = 30, int stepAngle = 10);
        void stopSpinning();

    protected:
        // Automatically re-draws when Qt calculates or changes widget geometry
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void onSpinTimerTimeout();

    private:
        void updateDisplay();

    private:
        QPixmap m_rawPixmap;
        QTimer m_spinTimer;
        int m_currentAngle = 0;
        int m_stepAngle    = 10;
        bool m_isSpinning  = false;
    };
}
