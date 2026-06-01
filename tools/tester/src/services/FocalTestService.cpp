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

        try
        {
            for (const auto &id : m_knownFocalIds)
            {
                auto focal = m_registry->get<HAL::Act::IFocal>(id);

                focal->disable();
            }
            m_status = Services::ServiceStatus::Success;
            m_errorReason.clear();
        }
        catch (const std::exception &e)
        {
            m_status      = Services::ServiceStatus::Error;
            m_errorReason = e.what();
            qCritical() << "[FocalTestService] Stop failed:" << e.what();
        }
    }

    // ==========================================
    // FOCAL OPERATIONS
    // ==========================================

    void FocalTestService::toggleFocal(const std::string &focalId, bool enabled)
    {
        qInfo() << "[FocalTestService] Toggling focal" << QString::fromStdString(focalId) << (enabled ? "ON" : "OFF");

        try
        {
            auto focal = m_registry->get<HAL::Act::IFocal>(focalId);

            if (enabled)
                focal->enable();
            else
                focal->disable();
            m_status = Services::ServiceStatus::Success;
        }
        catch (const std::exception &e)
        {
            m_status      = Services::ServiceStatus::Error;
            m_errorReason = e.what();
            qCritical() << "[FocalTestService] Toggle failed:" << e.what();
        }
    }

    void FocalTestService::updateFocalValue(const std::string &focalId, uint16_t value)
    {
        qInfo() << "[FocalTestService] Setting focal" << QString::fromStdString(focalId) << "to" << value;
        try
        {
            auto focal = m_registry->get<HAL::Act::IFocal>(focalId);

            focal->setValue(value);
            m_status = Services::ServiceStatus::Success;
        }
        catch (const std::exception &e)
        {
            m_status      = Services::ServiceStatus::Error;
            m_errorReason = e.what();
            qCritical() << "[FocalTestService] Update value failed:" << e.what();
        }
    }

    const std::vector<std::string> &FocalTestService::getAvailableFocals() const
    {
        return m_knownFocalIds;
    }

} // namespace Kub3::Tools::Tester
