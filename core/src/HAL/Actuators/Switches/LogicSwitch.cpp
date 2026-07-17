#include <HAL/Actuators/Switches/LogicSwitch.h>
#include <format>
#include <stdexcept>

namespace Kub3::HAL::Act
{

    LogicSwitch::LogicSwitch(std::string id, std::string onCmd, std::string offCmd, Weak<MCUDriver> driver) :
        m_id(std::move(id)),
        m_onCmd(QByteArray::fromStdString(onCmd)),
        m_offCmd(QByteArray::fromStdString(offCmd)),
        m_driver(std::move(driver)) {};

    void LogicSwitch::turnOn(void)
    {
        if (auto driver = m_driver.lock())
        {
            driver->sendCommand(m_onCmd);
            m_isOn = true;
        }
        else
        {
            throw std::runtime_error(std::format("Attempted to send command, but MCUDriver is dead. Actuator: '{}'", m_id));
        }
    }

    void LogicSwitch::turnOff(void)
    {
        if (auto driver = m_driver.lock())
        {
            driver->sendCommand(m_offCmd);
            m_isOn = false;
        }
        else
        {
            throw std::runtime_error(std::format("Attempted to send command, but MCUDriver is dead. Actuator: '{}'", m_id));
        }
    }

    void LogicSwitch::emergencyStop(void)
    {
        this->turnOff();
    }

    bool LogicSwitch::isOn(void) const
    {
        return m_isOn;
    }

    std::string_view LogicSwitch::getId(void) const noexcept
    {
        return m_id;
    }

    std::function<void(const QByteArray &)> LogicSwitch::createFeedbackHandler(Shared<LogicSwitch> sw)
    {
        return [weakSw = Weak<LogicSwitch>(sw)](const QByteArray &payload) {
            if (auto safeSw = weakSw.lock())
            {
                if (!payload.isEmpty())
                {
                    char byteVal = payload.at(0);

                    safeSw->m_isOn = (byteVal == 1 || byteVal == '1');
                }
            }
        };
    }

} // Kub3::HAL::Act
