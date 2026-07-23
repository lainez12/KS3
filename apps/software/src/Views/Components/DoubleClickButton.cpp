#include <Views/Components/DoubleClickButton.h>

DoubleClickButton::DoubleClickButton(const QString &text, QWidget *parent) :
    QPushButton(text, parent)
{
}

void DoubleClickButton::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Emit the doubleClicked signal when a double-click event occurs
    emit doubleClicked();

    // Call the base class implementation to ensure normal behavior
    QPushButton::mouseDoubleClickEvent(event);
}