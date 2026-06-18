#ifndef MESSAGEOVERLAY_h
#define MESSAGEOVERLAY_h
#include <QWidget>
#include <QPainter>
#include <QEvent>
#include <QResizeEvent>

class MessageOverlay : public QWidget {
    Q_OBJECT
public:
    explicit MessageOverlay(QWidget *parent = nullptr);
    void setText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    QString m_text;
};

#endif