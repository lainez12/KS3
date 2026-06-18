#pragma once

#include <string>
#include <string_view>

#include "IExposureSystem.h"
#include <HAL/MCUDriver.h>
#include <utils.h>

namespace Kub3::HAL::Act
{

    class UVExposureHead final : public IExposureSystem
    {
    public:
        UVExposureHead(std::string id, Weak<MCUDriver> driver);

        [[nodiscard]] std::string_view getId(void) const noexcept override;

        void startContinuousExposure(const ContinuousExposureParams &params) override;
        void startFlashingExposure(const FlashingExposureParams &params) override;
        void emergencyStop(void) override;

    private:
        void sendPayload(const QByteArray &payload) const;

    private:
        const std::string m_id;
        Weak<MCUDriver> m_driver;
    };

} // namespace Kub3::HAL::Act
