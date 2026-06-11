#include <Views/Components/MessageOverlay.h>

MessageOverlay::MessageOverlay(QWidget *parent) : QWidget(parent), m_text("...")
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

    painter.setPen(QColor(0, 102, 204));
    QFont font = painter.font();
    font.setPointSize(22);
    font.setBold(true);
    painter.setFont(font);

    painter.drawText(rect(), Qt::AlignCenter | Qt::TextWordWrap, m_text);
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