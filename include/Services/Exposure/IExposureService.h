#pragma once

#include <HAL/Actuators/Lights/UVExposureHead.h>
#include <Services/IService.h>
#include <variant>

namespace Kub3::Services
{
    using ExposurePayload = std::variant<
        HAL::Act::ContinuousExposureParams,
        HAL::Act::FlashingExposureParams>;

    class IExposureService : public IService
    {
    public:
        virtual ~IExposureService()                         = default;
        virtual void startExposure(ExposurePayload payload) = 0;
    };
}
