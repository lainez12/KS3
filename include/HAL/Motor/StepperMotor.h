#pragma once

#include <QString>

#include "HAL/Com/ICommunicator.h"
#include "HAL/Motor/IMotor.h"

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
