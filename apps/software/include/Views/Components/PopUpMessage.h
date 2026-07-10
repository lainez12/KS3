#ifndef PopUpMessage_H
#define PopUpMessage_H

#include <QEvent>
#include <QWidget>

#include <functional>

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QWidget;

class PopUpMessage : public QWidget
{
    Q_OBJECT

public:
    struct ButtonConfig {
        QString text;
        std::function<void()> callback;
    };

    explicit PopUpMessage(QWidget *parent = nullptr);
    PopUpMessage(const QString &title, const QString &message, const QVector<ButtonConfig> &buttons, QWidget *parent = nullptr);

    void setTitleText(const QString &title);
    void setMessageText(const QString &message);
    void setButtons(const QVector<ButtonConfig> &buttons);

    void showMessage();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    void buildUi();
    void refreshContent();
    void refreshButtons();

    QWidget *m_card              = nullptr;
    QWidget *m_header            = nullptr;
    QWidget *m_body              = nullptr;
    QWidget *m_buttonsContainer  = nullptr;
    QLabel *m_titleLabel         = nullptr;
    QLabel *m_messageLabel       = nullptr;
    QVBoxLayout *m_cardLayout    = nullptr;
    QHBoxLayout *m_headerLayout  = nullptr;
    QHBoxLayout *m_buttonsLayout = nullptr;

    QString m_titleText;
    QString m_messageText;
    QVector<ButtonConfig> m_buttons;
};

#endif