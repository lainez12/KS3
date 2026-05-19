#include <QLabel>
#include <QLayout>
#include <QPainter>

#include "Views/Components/UpBar.h"

#define TITLE_BAR "font-size: 40px; font-weight: bold;"

UpBar::UpBar(QWidget *parent) : QFrame(parent) {
    m_topBarTitle = new QLabel(this);
    m_topBarTitle->setAlignment(Qt::AlignCenter);
    
    if (auto layout = this->layout()) {
        QLabel *logoTopBar = new QLabel();
        logoTopBar->setPixmap(QPixmap(":/icons/logoKloeBlanc.svg").scaledToHeight(25));
        logoTopBar->setStyleSheet("background-color: #000");
        layout->addWidget(logoTopBar);
        layout->addWidget(m_topBarTitle);
    }
}

void UpBar::setTextColor(const QColor &color) {
}

void UpBar::setBackgroundColor(const QColor &color) {
}

void UpBar::setTitleBarConfig(Kub3::UI::Views::TitleBarConfig titleBar) {
    if (titleBar.showTitleBar) {
        // m_topBarTitle->setText(titleBar.viewTitle);
        m_topBarTitle->setText("e");
        m_topBarTitle->setStyleSheet(QString("color: %1;").arg(titleBar.textColor.name()));
        this->setStyleSheet(QString("%2 background-color: %1;").arg(titleBar.bgColor.name()).arg(TITLE_BAR));
    }
}

void UpBar::paintEvent(QPaintEvent *) {
    QPainter painter(this);
}