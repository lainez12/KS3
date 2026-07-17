// KeyboardConnections.cpp
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

        connectButton(parent, "btnQ" + suffix, Qt::Key_Q, "Q");
        connectButton(parent, "btnW" + suffix, Qt::Key_W, "W");
        connectButton(parent, "btnE" + suffix, Qt::Key_E, "E");
        connectButton(parent, "btnR" + suffix, Qt::Key_R, "R");
        connectButton(parent, "btnT" + suffix, Qt::Key_T, "T");
        connectButton(parent, "btnY" + suffix, Qt::Key_Y, "Y");
        connectButton(parent, "btnU" + suffix, Qt::Key_U, "U");
        connectButton(parent, "btnI" + suffix, Qt::Key_I, "I");
        connectButton(parent, "btnO" + suffix, Qt::Key_O, "O");
        connectButton(parent, "btnP" + suffix, Qt::Key_P, "P");

        connectButton(parent, "btnA" + suffix, Qt::Key_A, "A");
        connectButton(parent, "btnS" + suffix, Qt::Key_S, "S");
        connectButton(parent, "btnD" + suffix, Qt::Key_D, "D");
        connectButton(parent, "btnF" + suffix, Qt::Key_F, "F");
        connectButton(parent, "btnG" + suffix, Qt::Key_G, "G");
        connectButton(parent, "btnH" + suffix, Qt::Key_H, "H");
        connectButton(parent, "btnJ" + suffix, Qt::Key_J, "J");
        connectButton(parent, "btnK" + suffix, Qt::Key_K, "K");
        connectButton(parent, "btnL" + suffix, Qt::Key_L, "L");

        connectButton(parent, "btnZ" + suffix, Qt::Key_Z, "Z");
        connectButton(parent, "btnX" + suffix, Qt::Key_X, "X");
        connectButton(parent, "btnC" + suffix, Qt::Key_C, "C");
        connectButton(parent, "btnV" + suffix, Qt::Key_V, "V");
        connectButton(parent, "btnB" + suffix, Qt::Key_B, "B");
        connectButton(parent, "btnN" + suffix, Qt::Key_N, "N");
        connectButton(parent, "btnM" + suffix, Qt::Key_M, "M");

        connectButton(parent, "btnEspace" + suffix, Qt::Key_Space, " ");

        connectCapitalButton(parent, "btnShift" + suffix);
        // connectButton(parent, "btnShift" + suffix, Qt::Key_CapsLock, "");
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
                qDebug() << "Capital state toggled. New state: " << (isCapitalActive ? "Active" : "Inactive");
            });
        }
    }

    void KeyboardConnections::toggleCapitalState()
    {
        isCapitalActive = !isCapitalActive;
        if (isCapitalActive)
        {
            // siwtchToUpperCase();
        }
        else
        {
            switchToLowerCase();
        }
    }

    void KeyboardConnections::switchToUpperCase()
    {
        // switchLetterCase("btnQ", "Q", Qt::Key_Q);
    }

    void KeyboardConnections::switchToLowerCase()
    {
        // switchLetterCase("btnQ", "q", Qt::Key_Q);
    }

}