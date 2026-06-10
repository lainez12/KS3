#include <QThread>
#include <controllers/FocalTestController.h>

namespace Kub3::Tools::Tester
{

    FocalTestController::FocalTestController(Shared<HAL::Act::ActuatorRegistry> actuatorRegistry,
                                             std::vector<std::string> knownFocalIds,
                                             QObject *parent) :
        QObject(parent),
        m_actuatorRegistry(std::move(actuatorRegistry)),
        m_knownFocalIds(std::move(knownFocalIds))
    {
    }

    FocalTestController::~FocalTestController()
    {
        stop();
    }

    void FocalTestController::start()
    {
        // Instantiation inside the logic thread
        m_focalTestService = std::make_unique<FocalTestService>(m_actuatorRegistry, m_knownFocalIds);
    }

    void FocalTestController::stop()
    {
        if (m_focalTestService)
            m_focalTestService->stop();
    }

    void FocalTestController::ps_toggleFocal(const QString &focalId, bool enabled)
    {
        m_focalTestService->toggleFocal(focalId.toStdString(), enabled);
    }

    void FocalTestController::ps_updateFocalValue(const QString &focalId, uint16_t value)
    {
        m_focalTestService->updateFocalValue(focalId.toStdString(), value);
    }

    void FocalTestController::ps_emergencyStop()
    {
        m_focalTestService->stop();
    }

} // namespace Kub3::Tools::Tester
