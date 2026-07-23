#pragma once

#include <QPushButton>

class DoubleClickButton : public QPushButton
{
    Q_OBJECT
public:
    DoubleClickButton(const QString &text, QWidget *parent = nullptr);

signals:
    void doubleClicked();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;
};