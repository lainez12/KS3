#pragma once

#include <QByteArray>
#include <QObject>

namespace Kub3::HAL::Com
{

    class ICommunicator : public QObject
    {
        Q_OBJECT

    public:
        virtual ~ICommunicator() = default;

        virtual bool open()         = 0;
        virtual void close()        = 0;
        virtual bool isOpen() const = 0;
        // Send raw bytes to the hardware
        virtual bool send(QByteArray &&data) = 0;

    signals:
        void s_dataReceived(const QByteArray &data);
        void s_connected(void);
        void s_connectionLost(void);
    };

} // namespace Kub3::HAL
