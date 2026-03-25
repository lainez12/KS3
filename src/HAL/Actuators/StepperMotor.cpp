#include <QMetaObject>
#include <stdexcept>

#include "HAL/Actuators/StepperMotor.h"

namespace Kub3::HAL::Act
{

    StepperMotor::StepperMotor(std::string id, Weak<MCUDriver> driver) :
        m_id(std::move(id)),
        m_driver(std::move(driver))
    {
    }

    void StepperMotor::sendPayload(uint8_t *payload, uint32_t size) const
    {
        if (auto driver = m_driver.lock())
        {
            QMetaObject::invokeMethod(
                driver.get(),
                &MCUDriver::sendCommand,
                Qt::QueuedConnection,
                QByteArray(reinterpret_cast<char *>(payload), size));
        }
        else
        {
            throw std::runtime_error("Attempted to send command, but MCUDriver is dead. Actuator: " + m_id);
        }
    }

    void StepperMotor::moveAbsolute(int32_t position_mm)
    {
    }

    void StepperMotor::moveRelative(int32_t distance_mm)
    {
    }

    void StepperMotor::setTargetSpeed(uint32_t velocity_mm_s)
    {
    }

    void StepperMotor::emergencyStop(void)
    {
    }

    void StepperMotor::enable(bool state)
    {
    }

    void StepperMotor::home(void)
    {
    }

    bool StepperMotor::isMoving(void) const
    {
        return m_moving;
    }

    uint32_t StepperMotor::getTargetSpeed(void) const
    {
        return m_targetSpeed;
    }

} // namespace Kub3::HAL::Act
