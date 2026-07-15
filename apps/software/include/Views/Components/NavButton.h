#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>

#include "BottomCroppedCircle.h"

class NavButton : public QWidget
{
    Q_OBJECT
public:
    explicit NavButton(QWidget *parent = nullptr);

    void setup(const QString &text, const QColor &colorEnabled, const QColor &colorDisabled, const QString &iconPath = "", const QFont &font = QFont("Arial", 12));
    void setIcon(const QString &iconPath);
    void setText(const QString &newText);
    void setEnabledNavButton(bool state);
    void setSize(const uint sizePx);
    void changeColorToDisabled();
    void changeColorToEnabled();
    void switchColor(bool enabled);

signals:
    void clicked(void);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    BottomCroppedCircle *m_circle;
    QColor colorEnabled;
    QColor colorDisabled;
    QLabel *m_iconLabel;
    QLabel *m_text;
};