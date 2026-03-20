#include <QDebug>
#include <QSerialPortInfo>
#include <QThread>

#include "HAL/Com/SerialCommunicator.h"

namespace Kub3::HAL::Com
{
    SerialCommunicator::SerialCommunicator(const QString &portName, size_t baudRate) :
        m_portName(portName),
        m_baudRate(baudRate),
        m_pingTimer(this),
        m_serialPort(this)
    {
        m_serialPort.setPortName(m_portName);
        m_serialPort.setBaudRate(m_baudRate);
        m_serialPort.setDataBits(QSerialPort::Data8);
        m_serialPort.setParity(QSerialPort::NoParity);
        m_serialPort.setStopBits(QSerialPort::OneStop);
        m_serialPort.setFlowControl(QSerialPort::NoFlowControl);

        _setupConnections();

        // TODO: replace with a more event-driven approach if possible
        connect(&m_pingTimer, &QTimer::timeout, this, &SerialCommunicator::_verifyPortConnection);
    }

    SerialCommunicator::~SerialCommunicator()
    {
        this->close();
    }

    bool SerialCommunicator::open()
    {
        if (m_serialPort.isOpen())
            return true;

        if (!m_serialPort.open(QIODevice::ReadWrite))
        {
            qCritical().nospace() << "[Serial] Failed to open port " << m_portName << ": " << m_serialPort.errorString();
            return false;
        }

        m_serialPort.clear();
        m_pingTimer.start(1000);
        qInfo() << "[Serial] Successfully opened port" << m_portName << "at" << m_baudRate << "baud.";
        return true;
    }

    void SerialCommunicator::close()
    {
        if (!m_serialPort.isOpen())
            return;

        m_pingTimer.stop();
        m_serialPort.close();
        qInfo() << "[Serial] Port" << m_portName << "closed.";
    }

    bool SerialCommunicator::isOpen() const
    {
        return m_serialPort.isOpen();
    }

    bool SerialCommunicator::send(QByteArray &&data)
    {
        if (!isOpen())
        {
            qWarning() << "[Serial] Attempted to send data to closed port" << m_portName;
            return false;
        }

        const qint64 bytesWritten = m_serialPort.write(data);

        if (bytesWritten == -1)
        {
            qCritical() << "[Serial] Write error on" << m_portName << ":" << m_serialPort.errorString();
            return false;
        }
        // Ensure all bytes are actually pushed to the wire
        else if (bytesWritten != data.size())
        {
            qWarning() << "[Serial] Incomplete write. Expected:" << data.size() << "Written:" << bytesWritten;
        }

        m_serialPort.waitForBytesWritten(100); // 100ms timeout
        return true;
    }

    // Private methods

    void SerialCommunicator::_setupConnections(void)
    {
        // Handle incoming data
        connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialCommunicator::_onReadyRead);
        // Handle critical hardware disconnections (e.g., USB unplugged)
        connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialCommunicator::_onErrorOccurred);
    }

    void SerialCommunicator::_verifyPortConnection(void)
    {
        if (!m_serialPort.isOpen())
            return;

        bool found = false;

        for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        {
            if (info.portName() == m_serialPort.portName())
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            qCritical() << "[Serial] Connection to port" << m_serialPort.portName() << "lost.";
            m_serialPort.close();
            emit connectionLost();
        }
    }

    void SerialCommunicator::_onReadyRead(void)
    {
        if (!m_serialPort.isOpen())
            return;

        const QByteArray rawData = m_serialPort.readAll();

        if (!rawData.isEmpty())
        {
            emit dataReceived(rawData);
        }
    }

    void SerialCommunicator::_onErrorOccurred(QSerialPort::SerialPortError error)
    {
        // ResourceError = The device disappeared from the OS
        if (error == QSerialPort::ResourceError)
        {
            qCritical() << "[Serial] Critical Hardware Loss on" << m_portName << "Error:" << m_serialPort.errorString();
            this->close();         // Safely reset state
            emit connectionLost(); // Alert the subscribers
        }
        else if (error != QSerialPort::NoError)
        {
            qWarning() << "[Serial] Port Error on" << m_portName << ":" << m_serialPort.errorString();
        }
    }
}
