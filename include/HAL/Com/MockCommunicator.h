#pragma once

#include "HAL/Com/ICommunicator.h"

namespace Kub3::HAL::Com
{

    class MockCommunicator : public ICommunicator
    {
    public:
        MockCommunicator()  = default;
        ~MockCommunicator() = default;

        bool open() override final;
        void close() override final;
        bool isOpen() const override final;
        bool send(QByteArray &&data) override final;

        void fakeReceive(QByteArray &&data);

    public:
        QByteArray lastSentData; // Buffer to save the exact bytes attempted to be "sent"

    private:
        bool m_isOpen = false;
    };

}
