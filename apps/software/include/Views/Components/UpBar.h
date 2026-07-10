#pragma once

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>
#include <QWidget>

#include <Views/ViewBase.h>

class UpBar : public QFrame
{
    Q_OBJECT
public:
    explicit UpBar(QWidget *parent);

    void setTextColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void setTitleBarConfig(Kub3::UI::Views::TitleBarConfig titleBar);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateTitlePosition();

private:
    QHBoxLayout *layoutIntern = nullptr;
    QLabel *m_logoTopBar      = nullptr;
    QLabel *m_topBarTitle     = nullptr;
    QLabel *m_iconPath        = nullptr;
    QLabel *m_sectionTitle    = nullptr;
    QPixmap *logoImage;
};