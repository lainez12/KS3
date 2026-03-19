#include "HAL/Com/MockCommunicator.h"

namespace Kub3::HAL::Com
{

    bool MockCommunicator::open(void)
    {
        m_isOpen = true;
        return true;
    }

    void MockCommunicator::close(void)
    {
        m_isOpen = false;
    }

    bool MockCommunicator::isOpen(void) const
    {
        return m_isOpen;
    }

    bool MockCommunicator::send(QByteArray &&data)
    {
        lastSentData = data; // Store last sent data
        return true;
    }

    void MockCommunicator::fakeReceive(QByteArray &&data)
    {
        emit dataReceived(data);
    }

}
