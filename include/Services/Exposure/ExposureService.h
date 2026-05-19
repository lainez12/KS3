#pragma once

#include "IExposureService.h"
#include <HAL/Actuators/ActuatorRegistry.h>

namespace Kub3::Services
{
    class ExposureService final : public IExposureService
    {
    public:
        ExposureService(Shared<HAL::Act::ActuatorRegistry> registry);

        void tick(void) override;
        void stop(void) override;
        [[nodiscard]] ServiceStatus getStatus(void) const noexcept override
        {
            return m_status;
        }
        [[nodiscard]] std::string getErrorReason(void) const override
        {
            return "";
        }

        void startExposure(ExposurePayload payload) override;

    private:
        Shared<HAL::Act::UVExposureHead> m_uvHead;
        ServiceStatus m_status    = ServiceStatus::Idle;
        uint32_t m_remainingTicks = 0;
    };
}
