#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QResizeEvent>

#include <Views/Components/UpBar.h>

#define TITLE_BAR     "font-size: 60px; font-weight: 700; font-family: arial; letter-spacing: 2px;"
#define TITLE_SECTION "font-size: 24px; font-weight: bold; font-family: arial;"
#define PATH_LOGO     ":/icons/logoKloeBlanc.svg"

UpBar::UpBar(QWidget *parent) : QFrame(parent)
{
    layoutIntern = new QHBoxLayout(this);
    layoutIntern->setContentsMargins(50, 0, 45, 0);

    QHBoxLayout *leftLayout = new QHBoxLayout();
    m_logoTopBar            = new QLabel(this);
    logoImage               = new QPixmap(PATH_LOGO);
    m_logoTopBar->setPixmap(*logoImage);
    m_logoTopBar->setScaledContents(true);
    leftLayout->addWidget(m_logoTopBar);

    m_topBarTitle = new QLabel(this);
    m_topBarTitle->setAlignment(Qt::AlignCenter);
    m_topBarTitle->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_topBarTitle->raise();

    QHBoxLayout *rightLayout = new QHBoxLayout();
    m_iconPath               = new QLabel(this);
    m_iconPath->setScaledContents(true);
    m_sectionTitle = new QLabel(this);
    rightLayout->addWidget(m_iconPath);
    rightLayout->addWidget(m_sectionTitle);
    rightLayout->setSpacing(16);

    layoutIntern->setContentsMargins(QMargins(40, 0, 40, 0));
    layoutIntern->addLayout(leftLayout);
    layoutIntern->addStretch(1);
    layoutIntern->addLayout(rightLayout);
}

void UpBar::updateTitlePosition()
{
    if (m_topBarTitle)
    {
        m_topBarTitle->adjustSize();

        int x = (this->width() - m_topBarTitle->width()) / 2;
        int y = (this->height() - m_topBarTitle->height()) / 2;
        m_topBarTitle->move(x, y);
    }
}

void UpBar::setTextColor(const QColor &color)
{
    m_topBarTitle->setStyleSheet(QString("%2 color: %1;").arg(color.name()).arg(TITLE_BAR));
    m_sectionTitle->setStyleSheet(QString("%2 color: %1; font-size: 24px;").arg(color.name()).arg(TITLE_SECTION));
    updateTitlePosition();
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
    updateTitlePosition();
}

void UpBar::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);

    if (m_topBarTitle != nullptr)
    {
        m_topBarTitle->setGeometry(0, 0, width(), height());
    }
    updateTitlePosition();
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