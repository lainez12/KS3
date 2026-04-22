#pragma once

#include <QMetaObject>

namespace Kub3::UI::ViewModels
{

    class BaseViewModel
    {
    public:
        ~BaseViewModel();

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

    private:
        std::vector<std::function<QMetaObject::Connection()>> m_connectionBuilders;
        std::vector<QMetaObject::Connection> m_activeConnections;
    };

} // Kub3::UI::ViewModels
