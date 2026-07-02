#include <QDebug>

#include <HAL/Actuators/Focal/Focal.h>

namespace Kub3::HAL::Act
{

    Focal::Focal(std::string id, uint8_t byteId, uint16_t maximumValue, Weak<MCUDriver> driver) :
        m_id(std::move(id)),
        m_byteId(byteId),
        m_maximumValue(maximumValue),
        m_driver(std::move(driver)) {}

    void Focal::enable()
    {
        m_enabled = true;
        this->sendCommand();
    }

    void Focal::disable()
    {
        m_enabled = false;
        this->sendCommand();
    }

    void Focal::setValueFraction(double val)
    {
        m_valueFraction = std::clamp(val, 0.0, 1.0);
        this->sendCommand();
    }

    void Focal::emergencyStop(void)
    {
        this->disable();
    }

    void Focal::sendCommand(void)
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
        command.append('F');
        command.append(enableByte);
        command.append(QByteArray::number(static_cast<uint32_t>(m_valueFraction * m_maximumValue)));

        qDebug() << "Focal sending (Hex):" << command.toHex(' ') << "| (ASCII):" << command;
        driver->sendCommand(command);
    }
}
