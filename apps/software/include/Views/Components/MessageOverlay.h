#ifndef MESSAGEOVERLAY_h
#define MESSAGEOVERLAY_h
#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

class MessageOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit MessageOverlay(QWidget *parent = nullptr);
    void setText(const QString &text);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    QString m_text;
    bool warning = false;
    QString warningText;
};

#endif