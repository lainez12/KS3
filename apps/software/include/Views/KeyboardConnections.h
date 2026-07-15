// KeyboardConnections.h
#pragma once

#include <QPointer>
#include <QPushButton>
#include <QWidget>
#include <Views/ViewBase.h>
#include <memory>

namespace Kub3::UI::Views
{

    class KeyboardConnections : public QWidget
    {
        Q_OBJECT
    public:
        explicit KeyboardConnections(QWidget *parent = nullptr);
        virtual ~KeyboardConnections() = default;

        void simulationKey(Qt::Key keyCode, const QString &text);
        void clearInputSelected();
        void setupKeyboardConnections(QWidget *parent = nullptr, const QString &suffix = "");

    protected:
    private:
        void updateLastEditableFocus(QWidget *newWidget);
        void connectButton(QWidget *parent, const QString &buttonName, Qt::Key keyCode, const QString &text);
        void connectClearButton(QWidget *parent, const QString &buttonName);

    private:
        QPointer<QWidget> m_lastEditableFocus;
    };

}