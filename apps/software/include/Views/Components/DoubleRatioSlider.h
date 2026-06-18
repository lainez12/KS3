#pragma once

#include <QApplication>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QtGui>

class DoubleRatioSlider : public QSlider
{
    Q_OBJECT

public:
    DoubleRatioSlider(QWidget *parent = nullptr);

signals:
    void doubleValueChanged(double value);

private slots:
    void notifyValueChanged(int value);
};
