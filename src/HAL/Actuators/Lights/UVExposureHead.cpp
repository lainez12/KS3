#include <QString>
#include <stdexcept>

#include "HAL/Actuators/Lights/UVExposureHead.h"

namespace Kub3::HAL::Act
{

    UVExposureHead::UVExposureHead(std::string id, Weak<MCUDriver> driver) :
        m_id(std::move(id)),
        m_driver(std::move(driver))
    {}

    std::string_view UVExposureHead::getId(void) const noexcept
    {
        return m_id;
    }

    void UVExposureHead::startContinuousExposure(const ContinuousExposureParams &params)
    {
#if defined(KUB_MODEL_8)
        // Format: IC<DUREE>#<PUISSANCE LEDS>#<PUISSANCE COURONNE>
        QString command = QString("IC%1#%2#%3")
                              .arg(params.durationMs)
                              .arg(params.centerPowerPct)
                              .arg(params.crownPowerPct);
#endif

        sendPayload(command.toUtf8());
    }

    void UVExposureHead::startFlashingExposure(const FlashingExposureParams &params)
    {
#if defined(KUB_MODEL_8)
        // Format: IF<NB CYCLE>#<DUREE>#<TEMPS PAUSE>#<PUISSANCE LEDS>#<PUISSANCE COURONNE>
        QString command = QString("IF%1#%2#%3#%4#%5")
                              .arg(params.cycles)
                              .arg(params.durationMs)
                              .arg(params.pauseTimeMs)
                              .arg(params.centerPowerPct)
                              .arg(params.crownPowerPct);
#endif

        sendPayload(command.toUtf8());
    }

    void UVExposureHead::emergencyStop(void)
    {
        sendPayload("S");
    }

    void UVExposureHead::sendPayload(const QByteArray &payload) const
    {
        if (auto driver = m_driver.lock())
        {
            QMetaObject::invokeMethod(
                driver.get(),
                &MCUDriver::ps_sendCommand,
                Qt::QueuedConnection,
                payload);
        }
        else
        {
            throw std::runtime_error("Attempted to send exposure command, but MCUDriver is dead. Actuator: " + m_id);
        }
    }

} // namespace Kub3::HAL::Act
