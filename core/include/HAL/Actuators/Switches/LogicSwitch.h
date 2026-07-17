#pragma once

#include <QByteArray>
#include <atomic>
#include <string>

#include "./ISwitch.h"
#include <HAL/MCUDriver.h>
#include <utils.h>

namespace Kub3::HAL::Act
{

    class LogicSwitch final : public ISwitch
    {
    public:
        LogicSwitch(std::string id, std::string onCmd, std::string offCmd, Weak<MCUDriver> driver);

        void turnOn(void) override;
        void turnOff(void) override;
        void emergencyStop(void) override;

        [[nodiscard]] bool isOn(void) const override;
        [[nodiscard]] std::string_view getId(void) const noexcept override;

        static std::function<void(const QByteArray &)> createFeedbackHandler(Shared<LogicSwitch> sw);

    private:
        const std::string m_id;
        const QByteArray m_onCmd;
        const QByteArray m_offCmd;

        Weak<MCUDriver> m_driver;
        std::atomic<bool> m_isOn = false;
    };

} // Kub3::HAL::Act
