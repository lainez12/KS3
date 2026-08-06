#include <QDebug>

#include <Common/Clock.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <Services/Exposure/ExposureService.h>
#include <utils.h>

namespace Kub3::Services
{

    ExposureService::ExposureService(Shared<HAL::Act::ActuatorRegistry> registry)
    {
        UNWRAP_OR_THROW(uvHead, registry->get<HAL::Act::UVExposureHead>(UV_EXPOSURE_HEAD), "[ExposureService] ");
        m_uvHead = uvHead;
    }

    void ExposureService::tick(void)
    {
        if (m_status != ServiceStatus::Running)
            return;

        if (m_remainingTicks > 0)
        {
            m_remainingTicks--;
            if (m_remainingTicks == 0)
                m_status = ServiceStatus::Success; // Exposure complete: update status
        }
    }

    void ExposureService::stop(void)
    {
        if (m_uvHead)
            m_uvHead->emergencyStop();
        m_remainingTicks = 0;
        m_status         = ServiceStatus::Idle;
    }

    void ExposureService::startExposure(ExposurePayload payload)
    {
        if (m_status == ServiceStatus::Running)
            return;

        m_status = ServiceStatus::Running;

        const auto museum = overloadedCallable{
            [&](const HAL::Act::ContinuousExposureParams &p) {
                m_uvHead->startContinuousExposure(p);
                m_remainingTicks = (p.durationMs / LOGIC_TIMER_PERIOD_MS) + 5; // Add 5 ticks (100ms) safety margin
            },
            [&](const HAL::Act::FlashingExposureParams &p) {
                m_uvHead->startFlashingExposure(p);
                const uint32_t totalMs = p.cycles * (p.durationMs + p.pauseTimeMs);
                m_remainingTicks       = (totalMs / LOGIC_TIMER_PERIOD_MS) + 5;
            }};

        std::visit(museum, payload);
    }

}
