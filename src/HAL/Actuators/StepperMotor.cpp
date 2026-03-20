#include "HAL/Actuators/StepperMotor.h"

namespace Kub3::HAL
{
    StepperMotor::StepperMotor(QObject *parent) : IMotor(parent)
    {
        // TODO: dependency injection of the encoder value ?
    }

    bool StepperMotor::initialize(void)
    {
        // TODO: determine if necessary
        return true;
    }

    void StepperMotor::moveTo(double position)
    {
        // TODO: setup proper "move to" command
        QString command = QString("MOVE_TO %1\n").arg(position);

        emit requestSend(command.toUtf8());
    }
}
