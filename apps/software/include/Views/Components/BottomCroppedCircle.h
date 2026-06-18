#pragma once

#include <QWidget>

class BottomCroppedCircle : public QWidget
{
    Q_OBJECT
public:
    explicit BottomCroppedCircle(QWidget *parent);

    void setColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QColor m_color = Qt::black;
};