#pragma once

#include <QColor>
#include <QFont>
#include <QLabel>
#include <QMouseEvent>
#include <QString>
#include <QWidget>

#include "BottomCroppedCircle.h"

class NavButton : public QWidget
{
    Q_OBJECT
public:
    struct SetupParams {
        QString text;
        QColor colorEnabled;
        QColor colorDisabled;
        QString iconPath    = "";
        QFont font          = QFont("Arial", 12);
        uint gapPx          = 0;
        QString textBgColor = "";
    };

    explicit NavButton(QWidget *parent = nullptr);

    void setup(const SetupParams &params);
    void setIcon(const QString &iconPath);
    void setText(const QString &newText);
    void setEnabledNavButton(bool state);
    void setSize(const uint sizePx);
    void setGap(const uint gapPx);
    void setTextColor(const QColor &color);
    void changeColorToDisabled();
    void changeColorToEnabled();
    void switchColor(bool active);

signals:
    void clicked(void);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    BottomCroppedCircle *m_circle;
    QColor colorEnabled;
    QColor colorDisabled;
    QString textBgColor;
    QColor textColor;
    QLabel *m_iconLabel;
    QLabel *m_text;
};