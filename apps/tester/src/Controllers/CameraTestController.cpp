#include <QThread>

#include <Controllers/CameraTestController.h>

namespace Kub3::Tools::Tester
{

    CameraTestController::CameraTestController(Shared<HAL::Act::ActuatorRegistry> actuatorRegistry,
                                               std::vector<std::string> knownFocalIds,
                                               std::vector<std::string> knownLightIds,
                                               QObject *parent) :
        QObject(parent),
        m_actuatorRegistry(std::move(actuatorRegistry)),
        m_knownFocalIds(std::move(knownFocalIds)),
        m_knownLightIds(std::move(knownLightIds))
    {
    }

    CameraTestController::~CameraTestController()
    {
        stop();
    }

    void CameraTestController::start()
    {
        // Instantiated inside the logic thread
        m_focalTestService    = std::make_unique<FocalTestService>(m_actuatorRegistry, m_knownFocalIds);
        m_lightingTestService = std::make_unique<LightingTestService>(m_actuatorRegistry, m_knownLightIds);
    }

    void CameraTestController::stop()
    {
        if (m_focalTestService)
            m_focalTestService->stop();

        if (m_lightingTestService)
            m_lightingTestService->stop();
    }

    void CameraTestController::ps_toggleFocal(const QString &focalId, bool enabled)
    {
        m_focalTestService->toggleFocal(focalId.toStdString(), enabled);
    }

    void CameraTestController::ps_updateFocalValue(const QString &focalId, uint16_t value)
    {
        m_focalTestService->updateFocalValue(focalId.toStdString(), value);
    }

    void CameraTestController::ps_toggleLight(const QString &lightId, bool enabled)
    {
        m_lightingTestService->toggleLight(lightId.toStdString(), enabled);
    }

    void CameraTestController::ps_updateLightValue(const QString &lightId, uint16_t value)
    {
        m_lightingTestService->updateLightValue(lightId.toStdString(), value);
    }

    void CameraTestController::ps_emergencyStop()
    {
        if (m_focalTestService)
            m_focalTestService->stop();

        if (m_lightingTestService)
            m_lightingTestService->stop();
    }

} // namespace Kub3::Tools::Tester
