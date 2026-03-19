#pragma once

#include <QSerialPort>
#include <QTimer>

#include "HAL/Com/ICommunicator.h"

namespace Kub3::HAL::Com
{
    class SerialCommunicator : public ICommunicator
    {
    public:
        SerialCommunicator(const QString &portName, size_t baudrate);
        ~SerialCommunicator() override final;

        bool open() override final;
        void close() override final;
        bool isOpen() const override final;
        bool send(QByteArray &&data) override final;

    private:
        void _setupConnections(void);
        void _verifyPortConnection(void);

    private slots:
        void _onReadyRead(void);
        void _onErrorOccurred(QSerialPort::SerialPortError error);

    private:
        QSerialPort m_serialPort;
        QString m_portName;
        size_t m_baudRate;

        QTimer m_pingTimer;
    };
}
