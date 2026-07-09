#include "ui_NavButton.h"
#include <Views/Components/NavButton.h>

NavButton::NavButton(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0); // No gap between circle and text

    m_circle = new BottomCroppedCircle(this);
    m_circle->setFixedSize(60, 60); // Lock the size of the icon circle

    {
        QVBoxLayout *circleLayout = new QVBoxLayout(m_circle);
        circleLayout->setAlignment(Qt::AlignCenter);
        circleLayout->setContentsMargins(0, 0, 0, 2);

        m_iconLabel = new QLabel(m_circle);
        m_iconLabel->setAlignment(Qt::AlignCenter);
        m_iconLabel->setStyleSheet("color: white;");
        m_iconLabel->setScaledContents(true); // Allow image to scale
        m_iconLabel->setFixedSize(35, 35);    // Size of the white icon

        circleLayout->addWidget(m_iconLabel);
    }

    m_text = new QLabel(this);
    m_text->setAlignment(Qt::AlignCenter);
    m_text->setStyleSheet("color: #0072ba;");

    layout->addWidget(m_circle, 0, Qt::AlignHCenter);
    layout->addWidget(m_text, 0, Qt::AlignHCenter);
}

void NavButton::setup(const QString &text, const QColor &colorEnabled, const QColor &colorDisabled, const QString &iconPath, const QFont &font)
{
    m_text->setText(text);
    m_text->setFont(font);
    m_circle->setColor(colorEnabled);
    this->colorEnabled  = colorEnabled;
    this->colorDisabled = colorDisabled;

    m_text->setStyleSheet(QString("color: %1;").arg(colorEnabled.name()));
    if (!iconPath.isEmpty())
        this->setIcon(iconPath);
}

void NavButton::setIcon(const QString &path)
{
    QPixmap pix(path);

    if (!pix.isNull())
        m_iconLabel->setPixmap(pix);
}

void NavButton::setText(const QString &newText)
{
    m_text->setText(newText);
}

void NavButton::setEnabledNavButton(bool state)
{
    setEnabled(state);
    if (state)
    {
        m_text->setStyleSheet(QString("color: %1;").arg(colorEnabled.name()));
        m_circle->setColor(colorEnabled);
    }
    else
    {
        m_text->setStyleSheet(QString("color: %1;").arg(colorDisabled.name()));
        m_circle->setColor(colorDisabled);
    }
}

void NavButton::changeColorToDisabled()
{
    m_text->setStyleSheet(QString("color: %1;").arg(colorDisabled.name()));
    m_circle->setColor(colorDisabled);
}

void NavButton::changeColorToEnabled()
{
    m_text->setStyleSheet(QString("color: %1;").arg(colorEnabled.name()));
    m_circle->setColor(colorEnabled);
}

void NavButton::switchColor(bool enabled)
{
    if (enabled)
    {
        changeColorToEnabled();
    }
    else
    {
        changeColorToDisabled();
    }
}

void NavButton::mouseReleaseEvent(QMouseEvent *event)
{
    emit clicked();
}

void NavButton::setSize(const uint sizePx)
{
    int iconSizePx = static_cast<int>(sizePx * (2.0 / 3.0));

    m_circle->setFixedSize(QSize(sizePx, sizePx));
    m_iconLabel->setFixedSize(QSize(iconSizePx, iconSizePx));
}
