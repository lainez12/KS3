#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QWidget>

#include "BottomCroppedCircle.h"

class NavButton : public QWidget {
    Q_OBJECT
public:
    explicit NavButton(QWidget *parent = nullptr);

    void setup(const QString &text, const QColor &color, const QString &iconPath = "");
    void setIcon(const QString &iconPath);
    void setText(const QString &newText);
    void setEnabledNavButton(bool state);
    void setSize(const uint sizePx);

signals:
    void clicked(void);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    BottomCroppedCircle *m_circle;
    QLabel *m_iconLabel;
    QLabel *m_text;
};