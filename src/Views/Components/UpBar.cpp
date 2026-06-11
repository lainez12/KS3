#include <QLabel>
#include <QLayout>
#include <QPainter>

#include <Views/Components/UpBar.h>

#define TITLE_BAR     "font-size: 50px; font-weight: bold;"
#define TITLE_SECTION "font-size: 17px; font-weight: bold;"
#define PATH_LOGO     ":/icons/logoKloeBlanc.svg"

UpBar::UpBar(QWidget *parent) : QFrame(parent)
{
    layoutIntern = new QHBoxLayout(this);

    QHBoxLayout *leftLayout = new QHBoxLayout();
    m_logoTopBar            = new QLabel(this);
    logoImage               = new QPixmap(PATH_LOGO);
    m_logoTopBar->setPixmap(*logoImage);
    m_logoTopBar->setScaledContents(true);
    leftLayout->addWidget(m_logoTopBar);

    m_topBarTitle = new QLabel(this);

    QHBoxLayout *rightLayout = new QHBoxLayout();
    m_iconPath               = new QLabel(this);
    m_iconPath->setScaledContents(true);
    m_sectionTitle = new QLabel(this);
    rightLayout->addWidget(m_iconPath);
    rightLayout->addWidget(m_sectionTitle);

    layoutIntern->addLayout(leftLayout);
    layoutIntern->addStretch(1);
    layoutIntern->addWidget(m_topBarTitle);
    layoutIntern->addStretch(1);
    layoutIntern->addLayout(rightLayout);
}

void UpBar::setTextColor(const QColor &color)
{
    m_topBarTitle->setStyleSheet(QString("%2 color: %1;").arg(color.name()).arg(TITLE_BAR));
    m_sectionTitle->setStyleSheet(QString("%2 color: %1;").arg(color.name()).arg(TITLE_SECTION));
}

void UpBar::setBackgroundColor(const QColor &color)
{
    this->setStyleSheet(QString("background-color: %1;").arg(color.name()));
}

void UpBar::setTitleBarConfig(Kub3::UI::Views::TitleBarConfig titleBar)
{
    if (!titleBar.showTitleBar)
    {
        return;
    }
    this->setBackgroundColor(titleBar.bgColor);
    this->setTextColor(titleBar.textColor);
    m_topBarTitle->setText(titleBar.viewTitle);
    m_sectionTitle->setText(titleBar.sectionTitle);
    m_iconPath->setPixmap(QPixmap(titleBar.iconPath));
}

void UpBar::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int widthLogoKloe  = this->width() * 0.08;
    int heightLogoKloe = widthLogoKloe / (logoImage->width() / logoImage->height()); // Aspect ratio of the logo
    m_logoTopBar->setFixedSize(widthLogoKloe, heightLogoKloe);
    int widthIconPath = this->width() * 0.04;
    m_iconPath->setFixedSize(widthIconPath, widthIconPath);
}