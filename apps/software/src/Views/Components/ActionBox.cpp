#include <QRegion>
#include <QResizeEvent>

#include <Views/Components/ActionBox.h>

ActionBox::ActionBox(QWidget *parent) : BottomCroppedCircle(parent)
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_layout = new QVBoxLayout(this);

    m_layout->setAlignment(Qt::AlignCenter);
    m_layout->setSpacing(30);
    m_layout->setContentsMargins(20, 40, 20, 50);

    this->setFixedSize(320, 320); // Default size, can be adjusted as needed
    // Icon Placeholder
    m_icon = new QLabel(this);
    m_icon->setAlignment(Qt::AlignCenter);
    m_icon->setScaledContents(true);

    // Text Label
    m_text = new QLabel(this);
    m_text->setAlignment(Qt::AlignCenter);
    m_text->setWordWrap(true);
    // Fixed font size for simplicity, or adjust as needed
    m_text->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");

    m_layout->addWidget(m_icon, 0, Qt::AlignHCenter);
    m_layout->addWidget(m_text, 0, Qt::AlignHCenter);
}

void ActionBox::setup(const QString &text, const QColor &color, const QString &iconPath)
{
    m_text->setText(text);
    this->setColor(color);
    this->setIcon(iconPath);
}

void ActionBox::setIcon(const QString &iconPath)
{
    m_iconPath = iconPath;
    if (iconPath.isEmpty())
        return;

    QPixmap pix(iconPath);

    if (!pix.isNull())
    {
        const int iconSize = this->width() * 0.36; // 36% of width

        m_icon->setPixmap(pix);
        m_icon->setFixedSize(iconSize, iconSize);
    }
}

void ActionBox::resizeEvent(QResizeEvent *event)
{
    BottomCroppedCircle::resizeEvent(event);

    int side = this->width();

    if (side <= 0)
        return;

    const int spacing        = m_layout->spacing();
    const int fontSize       = qMax(int(side * 0.08), 18); // 8% of width (min. font size = 18px)
    const int iconSize       = side * 0.36;                // 36% of width
    const int remainingSpace = side - (fontSize + iconSize + spacing);
    const int topMargin      = remainingSpace * 0.40;
    const int bottomMargin   = remainingSpace * 0.60;

    m_layout->setContentsMargins(20, remainingSpace / 2, 20, remainingSpace / 2);
    m_text->setStyleSheet(QString("color: white; font-size: %1px; font-weight: bold;").arg(fontSize));
    m_icon->setFixedSize(iconSize, iconSize);

    if (m_iconPath.isEmpty())
        m_icon->setStyleSheet(QString("background-color: white; border-radius: %1px;").arg(iconSize / 2));
    else
        m_icon->setStyleSheet(QString("background-color: none; border-radius: 0px;"));
}

void ActionBox::mouseReleaseEvent(QMouseEvent *event)
{
    emit clicked();
}