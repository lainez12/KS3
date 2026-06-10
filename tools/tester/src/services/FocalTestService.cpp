#include <QDebug>

#include <HAL/Actuators/Focal/IFocal.h>
#include <services/FocalTestService.h>

namespace Kub3::Tools::Tester
{

    FocalTestService::FocalTestService(Shared<HAL::Act::ActuatorRegistry> registry, std::vector<std::string> knownFocalIds) :
        m_registry(std::move(registry)),
        m_knownFocalIds(std::move(knownFocalIds))
    {
    }

    void FocalTestService::tick(void)
    {
        /* NO-OP */
    }

    void FocalTestService::stop(void)
    {
        qWarning() << "[FocalTestService] Emergency Stop triggered.";

        for (const auto &id : m_knownFocalIds)
        {
            auto focal = m_registry->get<HAL::Act::IFocal>(id);

            if (focal)
                focal->disable();
            else
                qCritical().noquote() << "[FocalTestService] Failed to disable focal" << id;
        }
        m_status = Services::ServiceStatus::Success;
        m_errorReason.clear();
    }

    // ==========================================
    // FOCAL OPERATIONS
    // ==========================================

    void FocalTestService::toggleFocal(const std::string &focalId, bool enabled)
    {
        qInfo() << "[FocalTestService] Toggling focal" << QString::fromStdString(focalId) << (enabled ? "ON" : "OFF");
        auto focal = m_registry->get<HAL::Act::IFocal>(focalId);

        if (!!focal)
        {
            if (enabled)
                focal->enable();
            else
                focal->disable();
            m_status = Services::ServiceStatus::Success;
        }
        else
        {
            m_status      = Services::ServiceStatus::Error;
            m_errorReason = focal.unwrap_err();
            qCritical() << "[FocalTestService] Toggle failed:" << m_errorReason;
        }
    }

    void FocalTestService::updateFocalValue(const std::string &focalId, uint16_t value)
    {
        qInfo() << "[FocalTestService] Setting focal" << QString::fromStdString(focalId) << "to" << value;

        auto focal = m_registry->get<HAL::Act::IFocal>(focalId);

        if (focal)
        {
            focal->setValue(value);
            m_status = Services::ServiceStatus::Success;
        }
        else
        {
            m_status      = Services::ServiceStatus::Error;
            m_errorReason = focal.unwrap_err();
            qCritical().noquote() << "[FocalTestService] Update value failed:" << QString::fromStdString(m_errorReason);
        }
    }

    const std::vector<std::string> &FocalTestService::getAvailableFocals() const
    {
        return m_knownFocalIds;
    }

} // namespace Kub3::Tools::Tester
