#include <QDebug>

#include <HAL/Actuators/Lights/CameraLightingLed.h>

namespace Kub3::HAL::Act
{

    CameraLightingLed::CameraLightingLed(std::string id, uint8_t byteId, uint16_t maximumValue, Weak<MCUDriver> driver) :
        m_id(std::move(id)),
        m_byteId(byteId),
        m_maximumValue(maximumValue),
        m_driver(std::move(driver)) {}

    void CameraLightingLed::enable()
    {
        m_enabled = true;
        this->sendCommand();
    }

    void CameraLightingLed::disable()
    {
        m_enabled = false;
        this->sendCommand();
    }

    void CameraLightingLed::setValueFraction(double val)
    {
        m_valueFraction = std::clamp(val, 0.0, 1.0);
        this->sendCommand();
    }

    void CameraLightingLed::emergencyStop(void)
    {
        this->disable();
    }

    void CameraLightingLed::sendCommand(void)
    {
        auto driver = m_driver.lock();

        if (!driver)
        {
            throw std::runtime_error(std::format("Attempted to send command, but MCUDriver is dead. Actuator: '{}'", m_id));
        }

        const char enableByte = (m_enabled ? '1' : '0');
        QByteArray command;

        command.append('4');
        command.append(m_byteId);
        command.append('L');
        command.append(enableByte);
        command.append(QByteArray::number(static_cast<uint32_t>(m_valueFraction * m_maximumValue)));

        qDebug() << "CameraLightingLed sending (Hex):" << command.toHex(' ') << "| (ASCII):" << command;
        driver->sendCommand(command);
    }
}
