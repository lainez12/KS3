#pragma once

#include <QObject>

#include "HAL/MachineStatus/IMachineStatusRepo.h"

namespace Kub3::HAL::Sensors
{

    class ISensor : public QObject
    {
        Q_OBJECT
    public:
        explicit ISensor(QObject *parent = nullptr) : QObject(parent) {};
        virtual ~ISensor() = default;

        // Entry point for packet routing
        virtual void processData(const QByteArray &data) = 0;
    };

} // Kub3::HAL::Sensors
