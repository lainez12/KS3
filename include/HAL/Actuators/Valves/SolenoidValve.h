#pragma once

#include <QByteArray>
#include <stdint.h>
#include <string>

#include "IValve.h"
#include <HAL/MCUDriver.h>
#include <utils.h>

namespace Kub3::HAL::Act
{

    class SolenoidValve final : public IValve
    {
    public:
        SolenoidValve(std::string id, std::string openCmd, std::string closeCmd, Weak<MCUDriver> driver);

        void open(void) override;
        void close(void) override;
        void emergencyStop(void) override;

        [[nodiscard]] bool isOpen(void) const override;
        [[nodiscard]] std::string_view getId(void) const noexcept override;

    private:
        const std::string m_id;
        const QByteArray m_openCmd;
        const QByteArray m_closeCmd;

        Weak<MCUDriver> m_driver;
        bool m_isOpen = false;
    };

}
