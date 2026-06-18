#include <QDebug>

#include <Services/LightingTestService.h>
#include <stdexcept>

namespace Kub3::Tools::Tester
{

    LightingTestService::LightingTestService(Shared<HAL::Act::ActuatorRegistry> registry,
                                             std::vector<std::string> knownLightIds) :
        m_registry(std::move(registry)),
        m_knownLightIds(std::move(knownLightIds)) {}

    void LightingTestService::tick(void)
    {
        /* NO-OP */
    }

    void LightingTestService::stop(void)
    {
        qWarning() << "[LightingTestService] Emergency Stop triggered.";

        for (const auto &id : m_knownLightIds)
        {
            auto light = m_registry->get<HAL::Act::ILightingSystem>(id);

            if (light)
                light->disable();
            else
                qCritical().noquote() << "[LightingTestService] Failed to disable light" << id;
        }
        m_status = Services::ServiceStatus::Success;
        m_errorReason.clear();
    }

    void LightingTestService::toggleLight(const std::string &lightId, bool enabled)
    {
        qInfo() << "[LightingTestService] Toggling light" << QString::fromStdString(lightId) << (enabled ? "ON" : "OFF");
        auto light = m_registry->get<HAL::Act::ILightingSystem>(lightId);

        if (!!light)
        {
            if (enabled)
                light->enable();
            else
                light->disable();
            m_status = Services::ServiceStatus::Success;
        }
        else
        {
            m_status      = Services::ServiceStatus::Error;
            m_errorReason = light.unwrap_err();
            qCritical() << "[LightingTestService] Toggle failed:" << m_errorReason;
        }
    }

    void LightingTestService::updateLightValue(const std::string &lightId, uint16_t value)
    {
        qInfo() << "[LightingTestService] Setting light" << QString::fromStdString(lightId) << "to" << value;

        auto light = m_registry->get<HAL::Act::ILightingSystem>(lightId);

        if (light)
        {
            light->setValue(value);
            m_status = Services::ServiceStatus::Success;
        }
        else
        {
            m_status      = Services::ServiceStatus::Error;
            m_errorReason = light.unwrap_err();
            qCritical().noquote() << "[LightingTestService] Update value failed:" << QString::fromStdString(m_errorReason);
        }
    }

    const std::vector<std::string> &LightingTestService::getAvailableLights() const
    {
        return m_knownLightIds;
    }

} // namespace Kub3::Tools::Tester
