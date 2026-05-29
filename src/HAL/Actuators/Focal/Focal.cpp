#include <HAL/Actuators/Focal/Focal.h>

namespace Kub3::HAL::Act
{

    Focal::Focal(std::string id, uint8_t byteId, Weak<MCUDriver> driver) :
        m_id(std::move(id)),
        m_byteId(byteId),
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

    void Focal::setValue(uint16_t val)
    {
        m_value = val;
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

        const uint8_t enableByte = (m_enabled ? '1' : '0');
        const uint8_t data[]     = {
            '4',
            m_byteId,
            'F',
            enableByte,
            static_cast<uint8_t>((m_value >> 8) & 0xFF),
            static_cast<uint8_t>(m_value & 0xFF),
        };
        QByteArray command(reinterpret_cast<const char *>(data), sizeof(data));

        driver->sendCommand(command);
    }
}
