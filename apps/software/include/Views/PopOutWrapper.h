#pragma once

#include <QCloseEvent>
#include <QPainter>
#include <QWidget>

#include <Views/MainWindow.h>

class PopOutWrapper : public QWidget
{
public:
    PopOutWrapper(Kub3::UI::ViewId id, Kub3::UI::Views::ViewBase *view, const QPixmap &bg, MainWindow *mainWin) :
        QWidget(mainWin, Qt::Window),
        m_viewId(id),
        m_view(view),
        m_bg(bg),
        m_mainWin(mainWin)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0); // View fills the entire window
        layout->addWidget(view);

        view->show();

        setWindowTitle(view->getTitleBar().viewTitle);
        resize(1024, 768);                  // Default size, adjust if needed
        setAttribute(Qt::WA_DeleteOnClose); // Qt handles memory when window closes
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        // Exact same styling as MainWindow
        QPainter painter(this);
        int scaledWidth = this->width() * 0.9;
        QPixmap scaled  = m_bg.scaledToWidth(scaledWidth, Qt::SmoothTransformation);
        int x           = this->width() - scaled.width();
        int y           = this->height() - scaled.height();

        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.fillRect(this->rect(), Qt::white);
        painter.drawPixmap(x, y, scaled);
    }

    void closeEvent(QCloseEvent *event) override
    {
        if (m_mainWin)
        {
            // Restore the view safely to the main stack before this wrapper deletes itself
            m_mainWin->ps_restoreView(m_viewId);
        }
        event->accept();
    }

private:
    Kub3::UI::ViewId m_viewId;
    Kub3::UI::Views::ViewBase *m_view;
    QPixmap m_bg;
    MainWindow *m_mainWin;
};
