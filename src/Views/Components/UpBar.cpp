#include <QLabel>
#include <QLayout>
#include <QPainter>

#include "Views/Components/UpBar.h"

#define TITLE_BAR "font-size: 40px; font-weight: bold;"

UpBar::UpBar(QWidget *parent) : QFrame(parent) {
    m_topBarTitle = new QLabel(this);
    m_logoTopBar  = new QLabel(this);
    m_logoTopBar->setPixmap(QPixmap(":/icons/logoKloeBlanc.svg"));
}

void UpBar::setTextColor(const QColor &color) {
}

void UpBar::setBackgroundColor(const QColor &color) {
    this->setStyleSheet(QString("%2 background-color: %1;").arg(color.name()).arg(TITLE_BAR));
}

void UpBar::setTitleBarConfig(Kub3::UI::Views::TitleBarConfig titleBar) {
    if (!titleBar.showTitleBar) {
        return;
    }
    this->setBackgroundColor(titleBar.bgColor);
    m_topBarTitle->setText(titleBar.viewTitle);
    m_topBarTitle->setAlignment(Qt::AlignCenter);
    m_topBarTitle->setStyleSheet(QString("color: %1;").arg(titleBar.textColor.name()));
}

void UpBar::paintEvent(QPaintEvent *) {
    QPainter painter(this);
}