#pragma once

#include <Common/Enums.h>
#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

namespace Ui
{
    class RealPositionCameras;
} // namespace Ui

class RealPositionCameras : public QWidget
{
    Q_OBJECT
public:
    explicit RealPositionCameras(QWidget *parent = nullptr);
    ~RealPositionCameras() override;

public:
    void openMap(void);
    void closeMap(void);

signals:
    void s_openMap(void);
    void s_closeMap(void);

public slots:
    void onCameraPositionUpdate(Kub3::CameraId camId, Kub3::CameraAxis axis, double value);

protected:
    void resizeEvent(QResizeEvent *ev) override;

private slots:
    void onBtnOpenCloseToggled(bool checked);

private:
    void updateRightXPosition(double x);
    void updateRightYPosition(double y);
    void updateLeftXPosition(double x);
    void updateLeftYPosition(double y);

    void updateCameraPositions();

private:
    Ui::RealPositionCameras *ui;
    QString m_text;
    QPixmap m_icon;

    // Store latest absolute position values (in mm)
    double m_leftX  = 0.0;
    double m_leftY  = 0.0;
    double m_rightX = 0.0;
    double m_rightY = 0.0;
};
