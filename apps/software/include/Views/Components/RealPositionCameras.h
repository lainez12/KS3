#pragma once

#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QResizeEvent>

namespace Ui
{
    class RealPositionCameras;
} // namespace Ui

class RealPositionCameras : public QWidget {
    Q_OBJECT
public:
    explicit RealPositionCameras(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::RealPositionCameras *ui;
    QString m_text;
};

