#pragma once

#include <QString>

#include "HAL/Actuators/IMotor.h"
#include "HAL/Com/ICommunicator.h"

namespace Kub3::HAL
{

    class StepperMotor : public IMotor
    {
    public:
        explicit StepperMotor(QObject *parent = nullptr);

        bool initialize(void) override final;
        void moveTo(double position) override final;
    };

} // namespace Kub3::HAL
