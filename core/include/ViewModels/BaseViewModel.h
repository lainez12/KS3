#pragma once

#include <QObject>

#include <Common/ProcessMessage.h>
#include <ViewModels/PopUpTypes.h>

namespace Kub3::UI::ViewModels
{

    class BaseViewModel : public QObject
    {
        Q_OBJECT

    public:
        BaseViewModel(QObject *parent = nullptr);
        virtual ~BaseViewModel();

        template <typename Sender, typename Signal, typename Slot>
        void bindConnection(Sender *sender, Signal signal, Slot slot, Qt::ConnectionType type = Qt::QueuedConnection)
        {
            m_connectionBuilders.push_back([sender, signal, receiver = this, slot, type]() {
                return QObject::connect(sender, signal, receiver, slot, type);
            });
        }

        template <typename Sender, typename Signal, typename Receiver, typename Slot>
        void bindConnection(Sender *sender, Signal signal, Receiver *receiver, Slot slot, Qt::ConnectionType type = Qt::QueuedConnection)
        {
            m_connectionBuilders.push_back([sender, signal, receiver, slot, type]() {
                return QObject::connect(sender, signal, receiver, slot, type);
            });
        }

        virtual void loadConnections(void);
        virtual void unloadConnections(void);

    signals:
        void s_showOverlayMsg(QString text);
        void s_createPopUp(const QString &title, const PopUpActions &buttons);
        void s_createPopUpWithText(const QString &title, const PopUpActions &buttons, const QString &initialText);
        void s_closePopUp();

    public slots:
        virtual void ps_onProcessMessageBroadcast(const Common::ProcessMessage &msg) {};

    private:
        std::vector<std::function<QMetaObject::Connection()>> m_connectionBuilders;
        std::vector<QMetaObject::Connection> m_activeConnections;
    };

} // Kub3::UI::ViewModels
