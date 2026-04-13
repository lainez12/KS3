#pragma once

#include "HAL/Actuators/IActuator.h"
#include <stdint.h>

namespace Kub3::HAL::Act
{

    struct ContinuousExposureParams {
        uint32_t durationMs;
#if defined(KUB_MODEL_8)
        uint8_t centerPowerPct; // 0-100
        uint8_t crownPowerPct;  // 0-100
#endif
    };

    struct FlashingExposureParams {
        uint32_t cycles;
        uint32_t durationMs;
        uint32_t pauseTimeMs;
#if defined(KUB_MODEL_8)
        uint8_t centerPowerPct; // 0-100
        uint8_t crownPowerPct;  // 0-100
#endif
    };

    class IExposureSystem : public IActuator
    {
    public:
        virtual ~IExposureSystem() = default;

        virtual void startContinuousExposure(const ContinuousExposureParams &params) = 0;
        virtual void startFlashingExposure(const FlashingExposureParams &params)     = 0;
    };

} // namespace Kub3::HAL::Act
