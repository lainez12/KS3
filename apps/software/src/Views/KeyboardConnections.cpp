#include <QApplication>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>

#include <Views/KeyboardConnections.h>

namespace Kub3::UI::Views
{

    namespace
    {
        static bool isEditableWidget(QWidget *widget)
        {
            return widget && (qobject_cast<QLineEdit *>(widget) || qobject_cast<QSpinBox *>(widget) || qobject_cast<QDoubleSpinBox *>(widget));
        }

        static QString widgetDescription(QWidget *widget)
        {
            if (!widget)
                return QStringLiteral("<null>");

            return QStringLiteral("%1 %2").arg(widget->metaObject()->className(), widget->objectName());
        }
    }

    KeyboardConnections::KeyboardConnections(QWidget *parent) :
        QWidget(parent)
    {
        if (qApp)
        {
            connect(qApp, &QApplication::focusChanged, this, [this](QWidget *, QWidget *newWidget) {
                updateLastEditableFocus(newWidget);
            });
        }
    }

    void KeyboardConnections::updateLastEditableFocus(QWidget *newWidget)
    {
        if (isEditableWidget(newWidget))
        {
            m_lastEditableFocus = newWidget;
        }
    }

    void KeyboardConnections::simulationKey(Qt::Key keyCode, const QString &text)
    {
        QWidget *focusedWidget = m_lastEditableFocus.data();
        if (focusedWidget && (qobject_cast<QLineEdit *>(focusedWidget) || qobject_cast<QSpinBox *>(focusedWidget) || qobject_cast<QDoubleSpinBox *>(focusedWidget)))
        {
            QKeyEvent *keyPress = new QKeyEvent(QEvent::KeyPress, keyCode, Qt::NoModifier, text);
            QApplication::postEvent(focusedWidget, keyPress);

            QKeyEvent *keyRelease = new QKeyEvent(QEvent::KeyRelease, keyCode, Qt::NoModifier, text);
            QApplication::postEvent(focusedWidget, keyRelease);
        }
        else
        {
            qDebug() << "No focused widget or focused widget is not a QLineEdit, QSpinBox, or QDoubleSpinBox.";
        }
    }

    void KeyboardConnections::clearInputSelected()
    {
        QWidget *focusedWidget = m_lastEditableFocus.data();
        if (!focusedWidget)
        {
            return;
        }
        if (QSpinBox *spinBox = qobject_cast<QSpinBox *>(focusedWidget))
        {
            spinBox->clear();
        }
        else if (QDoubleSpinBox *doubleSpinBox = qobject_cast<QDoubleSpinBox *>(focusedWidget))
        {
            doubleSpinBox->clear();
        }
        else if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(focusedWidget))
        {
            lineEdit->clear();
        }
    }

    void KeyboardConnections::setupKeyboardConnections(QWidget *parent, const QString &suffix)
    {
        if (!parent)
            parent = this;

        connectButton(parent, "btn0" + suffix, Qt::Key_0, "0");
        connectButton(parent, "btn1" + suffix, Qt::Key_1, "1");
        connectButton(parent, "btn2" + suffix, Qt::Key_2, "2");
        connectButton(parent, "btn3" + suffix, Qt::Key_3, "3");
        connectButton(parent, "btn4" + suffix, Qt::Key_4, "4");
        connectButton(parent, "btn5" + suffix, Qt::Key_5, "5");
        connectButton(parent, "btn6" + suffix, Qt::Key_6, "6");
        connectButton(parent, "btn7" + suffix, Qt::Key_7, "7");
        connectButton(parent, "btn8" + suffix, Qt::Key_8, "8");
        connectButton(parent, "btn9" + suffix, Qt::Key_9, "9");

        connectButton(parent, "btnBackspace" + suffix, Qt::Key_Backspace, "");
        connectClearButton(parent, "btnClear" + suffix);

        connectButton(parent, "btnEnter" + suffix, Qt::Key_Tab, "\t");

        switchToUpperCase();

        connectButton(parent, "btnEspace" + suffix, Qt::Key_Space, " ");

        connectCapitalButton(parent, "btnShift" + suffix);
    }

    void KeyboardConnections::connectButton(QWidget *parent, const QString &buttonName, Qt::Key keyCode, const QString &text)
    {
        QPushButton *btn = parent->findChild<QPushButton *>(buttonName);
        if (btn)
        {
            connect(btn, &QPushButton::clicked, this, [this, keyCode, text]() {
                simulationKey(keyCode, text);
            });
        }
    }

    void KeyboardConnections::connectClearButton(QWidget *parent, const QString &buttonName)
    {
        QPushButton *btn = parent->findChild<QPushButton *>(buttonName);
        if (btn)
        {
            connect(btn, &QPushButton::clicked, this, [this]() {
                clearInputSelected();
            });
        }
    }

    void KeyboardConnections::connectCapitalButton(QWidget *parent, const QString &buttonName)
    {
        QPushButton *btn = parent->findChild<QPushButton *>(buttonName);
        if (btn)
        {
            connect(btn, &QPushButton::clicked, this, [this]() {
                toggleCapitalState();
            });
        }
    }

    void KeyboardConnections::toggleCapitalState()
    {
        isCapitalActive = !isCapitalActive;
        if (isCapitalActive)
        {
            switchToUpperCase();
        }
        else
        {
            switchToLowerCase();
        }
    }

    void KeyboardConnections::switchToUpperCase()
    {
        for (int i = 0; i < 26; ++i)
        {
            char letter        = 'A' + i;
            QString buttonName = QString("btn%1").arg(letter);
            QPushButton *btn   = this->parent()->findChild<QPushButton *>(buttonName);
            if (btn)
            {

                btn->setText(QString(letter).toUpper());
                disconnect(btn, &QPushButton::clicked, this, nullptr);
                QWidget *parent = this->parentWidget();
                connectButton(parent, buttonName, Qt::Key(letter), QString(letter).toUpper());
            }
        }
    }

    void KeyboardConnections::switchToLowerCase()
    {
        for (int i = 0; i < 26; ++i)
        {
            char letter        = 'A' + i;
            QString buttonName = QString("btn%1").arg(letter);
            QPushButton *btn   = this->parent()->findChild<QPushButton *>(buttonName);
            if (btn)
            {
                btn->setText(QString(letter).toLower());
                disconnect(btn, &QPushButton::clicked, this, nullptr);
                QWidget *parent = this->parentWidget();
                connectButton(parent, buttonName, Qt::Key(letter), QString(letter).toLower());
            }
        }
    }

}