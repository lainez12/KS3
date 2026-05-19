#pragma once

#include <QFrame>
#include <QWidget>
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
    QHBoxLayout *layoutIntern = nullptr;
    QLabel *m_logoTopBar   = nullptr;
    QLabel *m_topBarTitle  = nullptr;
    QLabel *m_iconPath     = nullptr;
    QLabel *m_sectionTitle = nullptr;
};