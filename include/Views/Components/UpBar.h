#pragma once

#include <QWidget>
#include <QFrame>
#include <Views/ViewBase.h>

class UpBar : public QFrame {
    Q_OBJECT
public:
    explicit UpBar(QWidget *parent);

    void setTextColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void setTitleBarConfig(Kub3::UI::Views::TitleBarConfig titleBar);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QColor m_color        = Qt::black;
    QLabel *m_topBarTitle = nullptr;
};