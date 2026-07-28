#pragma once

#include <QEvent>
#include <QTextBrowser>
#include <QWidget>

#include <ViewModels/PopUpTypes.h>

class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QWidget;

class PopUpMessage : public QWidget
{
    Q_OBJECT

public:
    explicit PopUpMessage(QWidget *parent = nullptr);
    PopUpMessage(const QString &title, const QString &message, const PopUpActions &buttons, QWidget *parent = nullptr);

    void setTitleText(const QString &title);
    void setMessageText(const QString &message);
    void appendMessageText(const QString &text);
    void setButtons(const PopUpActions &buttons);

    void showMessage();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void hideEvent(QHideEvent *event) override;

private:
    void buildUI();
    void refreshContent();
    void refreshButtons();

private:
    QWidget *m_card                = nullptr;
    QWidget *m_header              = nullptr;
    QWidget *m_body                = nullptr;
    QWidget *m_buttonsContainer    = nullptr;
    QLabel *m_titleLabel           = nullptr;
    QTextBrowser *m_messageBrowser = nullptr;
    QVBoxLayout *m_cardLayout      = nullptr;
    QHBoxLayout *m_headerLayout    = nullptr;
    QHBoxLayout *m_buttonsLayout   = nullptr;

    QString m_titleText;
    QString m_messageText;
    PopUpActions m_buttons;
};
