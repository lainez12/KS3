#pragma once

#include <QObject>

namespace Kub3::HAL
{

    class IMotor : public QObject
    {
        Q_OBJECT

    public:
        explicit IMotor(QObject *parent = nullptr) : QObject(parent)
        {
        }
        virtual ~IMotor() = default;

        virtual bool initialize()            = 0;
        virtual void moveTo(double position) = 0;

    signals:
        void requestSend(QByteArray payload);
    };

} // Kub3::HAL
