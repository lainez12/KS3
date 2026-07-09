#include <Views/Components/PopMessage.h>

#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QSvgRenderer>
#include <QVBoxLayout>

namespace
{
    constexpr int kOverlayMargin = 36;
    constexpr int kCardMaxWidth  = 760;
    constexpr int kCardMinWidth  = 420;

    QPixmap renderSvgIcon(const QString &resourcePath, const QSize &size)
    {
        QPixmap pixmap(size);
        pixmap.fill(Qt::transparent);

        QSvgRenderer renderer(resourcePath);
        if (renderer.isValid())
        {
            QPainter painter(&pixmap);
            renderer.render(&painter, QRectF(QPointF(0, 0), QSizeF(size)));
        }

        return pixmap;
    }
}

PopMessage::PopMessage(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_StyledBackground, true);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    buildUi();

    if (parent)
    {
        parent->installEventFilter(this);
        resize(parent->size());
        raise();
    }

    hide();
}

PopMessage::PopMessage(const QString &title, const QString &message, const QVector<ButtonConfig> &buttons, QWidget *parent) : PopMessage(parent)
{
    setTitleText(title);
    setMessageText(message);
    setButtons(buttons);
}

void PopMessage::buildUi()
{
    setObjectName(QStringLiteral("PopMessageOverlay"));

    m_card = new QWidget(this);
    m_card->setObjectName(QStringLiteral("PopMessageCard"));
    m_card->setMinimumWidth(kCardMinWidth);
    m_card->setMaximumWidth(kCardMaxWidth);
    m_card->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto *shadow = new QGraphicsDropShadowEffect(m_card);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 10);
    shadow->setColor(QColor(0, 0, 0, 50));
    m_card->setGraphicsEffect(shadow);

    m_cardLayout = new QVBoxLayout(m_card);
    m_cardLayout->setContentsMargins(0, 0, 0, 0);
    m_cardLayout->setSpacing(0);

    m_header = new QWidget(m_card);
    m_header->setObjectName(QStringLiteral("PopMessageHeader"));
    m_header->setFixedHeight(78);

    m_headerLayout = new QHBoxLayout(m_header);
    m_headerLayout->setContentsMargins(18, 10, 18, 10);
    m_headerLayout->setSpacing(14);

    auto *leftIcon  = new QLabel(m_header);
    auto *rightIcon = new QLabel(m_header);
    leftIcon->setFixedSize(50, 50);
    rightIcon->setFixedSize(50, 50);
    leftIcon->setPixmap(renderSvgIcon(QStringLiteral(":/icons/warning-fond-blanc.svg"), leftIcon->size()));
    rightIcon->setPixmap(renderSvgIcon(QStringLiteral(":/icons/warning-fond-blanc.svg"), rightIcon->size()));
    leftIcon->setScaledContents(true);
    rightIcon->setScaledContents(true);

    m_titleLabel = new QLabel(m_header);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setWordWrap(true);

    m_headerLayout->addWidget(leftIcon, 0, Qt::AlignVCenter);
    m_headerLayout->addWidget(m_titleLabel, 1);
    m_headerLayout->addWidget(rightIcon, 0, Qt::AlignVCenter);

    m_body           = new QWidget(m_card);
    auto *bodyLayout = new QVBoxLayout(m_body);
    bodyLayout->setContentsMargins(28, 28, 28, 20);
    bodyLayout->setSpacing(22);

    m_messageLabel = new QLabel(m_body);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setWordWrap(true);

    m_buttonsContainer = new QWidget(m_body);
    m_buttonsLayout    = new QHBoxLayout(m_buttonsContainer);
    m_buttonsLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonsLayout->setSpacing(18);
    m_buttonsLayout->setAlignment(Qt::AlignCenter);

    bodyLayout->addWidget(m_messageLabel);
    bodyLayout->addWidget(m_buttonsContainer);

    m_cardLayout->addWidget(m_header);
    m_cardLayout->addWidget(m_body);

    m_card->setStyleSheet(
        "#PopMessageCard { background: white; border-radius: 10px; }"
        "#PopMessageHeader { background: #1976D2; border-top-left-radius: 10px; border-top-right-radius: 10px; }"
        "QLabel { color: #1976D2; }"
        "QLabel#PopMessageTitle { color: white; }"
        "QPushButton { min-width: 160px; min-height: 68px; padding: 10px 22px; font-size: 22px; font-weight: 700; color: #1976D2; background: white; border: 2px solid #1976D2; border-radius: 4px; }"
        "QPushButton:hover { background: #E8F2FF; }"
        "QPushButton:pressed { background: #D7E9FF; }");

    m_titleLabel->setObjectName(QStringLiteral("PopMessageTitle"));
    m_titleLabel->setStyleSheet("color: white; font-size: 30px; font-weight: 700;");
    m_messageLabel->setStyleSheet("color: #1976D2; font-size: 28px; font-weight: 700;");

    refreshContent();
    refreshButtons();

    auto *overlayLayout = new QVBoxLayout(this);
    overlayLayout->setContentsMargins(kOverlayMargin, kOverlayMargin, kOverlayMargin, kOverlayMargin);
    overlayLayout->addStretch(1);
    overlayLayout->addWidget(m_card, 0, Qt::AlignHCenter);
    overlayLayout->addStretch(1);
}

void PopMessage::setTitleText(const QString &title)
{
    m_titleText = title;
    refreshContent();
}

void PopMessage::setMessageText(const QString &message)
{
    m_messageText = message;
    refreshContent();
}

void PopMessage::setButtons(const QVector<ButtonConfig> &buttons)
{
    m_buttons = buttons;
    refreshButtons();
}

void PopMessage::showMessage()
{
    if (parentWidget())
    {
        resize(parentWidget()->size());
        raise();
    }

    show();
}

void PopMessage::refreshContent()
{
    if (m_titleLabel)
    {
        m_titleLabel->setText(m_titleText);
    }

    if (m_messageLabel)
    {
        m_messageLabel->setText(m_messageText);
    }
}

void PopMessage::refreshButtons()
{
    if (!m_buttonsLayout)
    {
        return;
    }

    while (QLayoutItem *item = m_buttonsLayout->takeAt(0))
    {
        if (QWidget *widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }

    for (const auto &buttonConfig : m_buttons)
    {
        auto *button = new QPushButton(buttonConfig.text, m_buttonsContainer);
        connect(button, &QPushButton::clicked, this, [this, callback = buttonConfig.callback]() {
            if (callback)
            {
                callback();
            }
            hide();
        });
        m_buttonsLayout->addWidget(button);
    }

    if (m_buttons.isEmpty())
    {
        m_buttonsContainer->hide();
    }
    else
    {
        m_buttonsContainer->show();
    }
}

void PopMessage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(255, 255, 255, 220));
}

bool PopMessage::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == parent())
    {
        if (ev->type() == QEvent::Resize)
        {
            if (QWidget *overlayParent = parentWidget())
            {
                resize(overlayParent->size());
            }
        }
        else if (ev->type() == QEvent::ChildAdded)
        {
            raise();
        }
    }

    return QWidget::eventFilter(obj, ev);
}