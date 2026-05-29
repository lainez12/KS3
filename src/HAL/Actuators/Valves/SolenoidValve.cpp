#include <HAL/Actuators/Valves/SolenoidValve.h>

namespace Kub3::HAL::Act
{

    SolenoidValve::SolenoidValve(std::string id, std::string openCmd, std::string closeCmd, Weak<MCUDriver> driver) :
        m_id(std::move(id)),
        m_openCmd(QByteArray::fromStdString(openCmd)),
        m_closeCmd(QByteArray::fromStdString(closeCmd)),
        m_driver(std::move(driver)) {};

    void SolenoidValve::open(void)
    {
        if (auto driver = m_driver.lock())
        {
            driver->sendCommand(m_openCmd);
            m_isOpen = true;
        }
        else
        {
            throw std::runtime_error(std::format("Attempted to send command, but MCUDriver is dead. Actuator: '{}'", m_id));
        }
    }

    void SolenoidValve::close(void)
    {
        if (auto driver = m_driver.lock())
        {
            driver->sendCommand(m_closeCmd);
            m_isOpen = false;
        }
        else
        {
            throw std::runtime_error(std::format("Attempted to send command, but MCUDriver is dead. Actuator: '{}'", m_id));
        }
    }

    void SolenoidValve::emergencyStop(void)
    {
        this->close();
    }

    bool SolenoidValve::isOpen(void) const
    {
        return m_isOpen;
    }

    std::string_view SolenoidValve::getId(void) const noexcept
    {
        return m_id;
    }

}
