#include "ui_NavButton.h"
#include <Views/Components/NavButton.h>

NavButton::NavButton(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);

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
        int iconSizePx = static_cast<int>(m_circle->width() * (2.0 / 3.0));
        m_iconLabel->setFixedSize(iconSizePx, iconSizePx); // Size of the white icon

        circleLayout->addWidget(m_iconLabel);
    }

    m_text = new QLabel(this);
    m_text->setAlignment(Qt::AlignCenter);
    m_text->setStyleSheet("color: #0072ba;");

    layout->addWidget(m_circle, 0, Qt::AlignHCenter);
    layout->addWidget(m_text, 0, Qt::AlignHCenter);
}

void NavButton::setup(const SetupParams &params)
{
    QString text = params.text;
    if (!text.isEmpty())
    {
        text[0] = text[0].toUpper();
    }
    m_text->setText(text);
    m_text->setFont(params.font);
    m_circle->setColor(params.colorEnabled);
    this->colorEnabled  = params.colorEnabled;
    this->colorDisabled = params.colorDisabled;
    this->textColor     = params.colorEnabled;
    this->textBgColor   = params.textBgColor;
    m_text->setStyleSheet(QString("color: %1; background-color: %2; padding: 2px; border-radius: 5px;").arg(textColor.name()).arg(textBgColor));
    if (!params.iconPath.isEmpty())
        this->setIcon(params.iconPath);
    this->setGap(params.gapPx);
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
        m_text->setStyleSheet(QString("color: %1; background-color: %2; padding: 2px; border-radius: 5px;").arg(textColor.name()).arg(textBgColor));
        m_circle->setColor(colorEnabled);
    }
    else
    {
        m_text->setStyleSheet(QString("color: %1; background-color: %2; padding: 2px; border-radius: 5px;").arg(colorDisabled.name()).arg(textBgColor));
        m_circle->setColor(colorDisabled);
    }
}

void NavButton::changeColorToDisabled()
{
    m_text->setStyleSheet(QString("color: %1; background-color: %2; padding: 2px; border-radius: 5px;").arg(colorDisabled.name()).arg(textBgColor));
    m_circle->setColor(colorDisabled);
}

void NavButton::changeColorToEnabled()
{
    m_text->setStyleSheet(QString("color: %1; background-color: %2; padding: 2px; border-radius: 5px;").arg(textColor.name()).arg(textBgColor));
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

void NavButton::setGap(const uint gapPx)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    if (layout)
    {
        layout->setSpacing(gapPx);
    }
}

void NavButton::setTextColor(const QColor &color)
{
    textColor = color;
    m_text->setStyleSheet(QString("color: %1; background-color: %2; padding: 2px; border-radius: 5px;").arg(textColor.name()).arg(textBgColor));
}
