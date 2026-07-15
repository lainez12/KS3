#include <Views/Components/MessageOverlay.h>

#include <QSvgRenderer>

MessageOverlay::MessageOverlay(QWidget *parent) : QWidget(parent), m_text("..."), warning(true), warningText("We recommend you proceed a visual prealignment")
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);

    if (parent)
    {
        parent->installEventFilter(this);
        resize(parent->size());
        raise();
    }
    hide();
}

void MessageOverlay::setText(const QString &text)
{
    m_text = text;
    update();
}

void MessageOverlay::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(255, 255, 255, 220));

    const int margin        = 32;
    const int topTextHeight = qMin(120, height() / 2);

    painter.setPen(QColor(0, 102, 204));
    QFont font = painter.font();
    font.setPointSize(22);
    font.setBold(true);
    painter.setFont(font);

    const QRect topTextRect(margin, margin, width() - margin * 2, topTextHeight);
    painter.drawText(topTextRect, Qt::AlignCenter | Qt::TextWordWrap, m_text);

    if (!warning)
    {
        return;
    }

    const int panelMargin = 32;
    const int panelHeight = qMin(140, height() / 3);
    const int panelWidth  = qMax(260, static_cast<int>(width() * 0.4));
    const int panelX      = (width() - panelWidth) / 2;
    const int panelY      = qMin(height() - panelMargin - panelHeight, topTextRect.bottom() + panelMargin);
    const QRect panelRect(panelX, panelY, panelWidth, panelHeight);

    painter.setPen(QPen(QColor(255, 0, 0), 3));
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(panelRect, 12, 12);

    const int innerPadding = 18;
    const int iconSize     = qMin(72, panelHeight - innerPadding * 2);
    const QRectF iconRect(panelRect.left() + innerPadding,
                          panelRect.top() + (panelRect.height() - iconSize) / 2.0,
                          iconSize,
                          iconSize);

    QSvgRenderer warningIcon(QStringLiteral(":/icons/warning-fond-blanc.svg"));
    if (warningIcon.isValid())
    {
        warningIcon.render(&painter, iconRect);
    }

    painter.setPen(QColor(255, 0, 0));
    font.setPointSize(18);
    font.setBold(true);
    painter.setFont(font);

    const int textLeft = static_cast<int>(iconRect.right()) + innerPadding;
    const QRect warningTextRect(textLeft,
                                panelRect.top() + innerPadding,
                                panelRect.right() - textLeft - innerPadding,
                                panelRect.height() - innerPadding * 2);
    painter.drawText(warningTextRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, warningText);
}

bool MessageOverlay::eventFilter(QObject *obj, QEvent *ev)
{
    if (obj == parent())
    {
        if (ev->type() == QEvent::Resize)
        {
            resize(static_cast<QResizeEvent *>(ev)->size());
        }
        else if (ev->type() == QEvent::ChildAdded)
        {
            raise();
        }
    }
    return QWidget::eventFilter(obj, ev);
}