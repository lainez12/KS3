#include <Views/Components/DoubleRatioSlider.h>

DoubleRatioSlider::DoubleRatioSlider(QWidget *parent) : QSlider(parent)
{
    connect(this, &QSlider::valueChanged, this, &DoubleRatioSlider::notifyValueChanged);
}

void DoubleRatioSlider::notifyValueChanged(int value)
{
    emit doubleValueChanged(static_cast<double>(value) / this->maximum()); // Value between 0.0 and 1.0
}
