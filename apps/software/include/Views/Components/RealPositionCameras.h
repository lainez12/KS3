#pragma once

#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QResizeEvent>

class RealPositionCameras : public QWidget {
    Q_OBJECT
public:
    explicit RealPositionCameras(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_text;
};

