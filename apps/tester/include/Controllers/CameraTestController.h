#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <string>
#include <vector>

#include <HAL/Actuators/ActuatorRegistry.h>
#include <Services/FocalTestService.h>
#include <Services/LightingTestService.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{

    class CameraTestController : public QObject
    {
        Q_OBJECT

    public:
        explicit CameraTestController(Shared<HAL::Act::ActuatorRegistry> actuatorRegistry,
                                      std::vector<std::string> knownFocalIds,
                                      std::vector<std::string> knownLightIds,
                                      QObject *parent = nullptr);
        ~CameraTestController() override;

    public slots:
        // System lifecycle
        void start();
        void stop();

        // Commands from ViewModel - Focals
        void ps_toggleFocal(const QString &focalId, bool enabled);
        void ps_updateFocalValue(const QString &focalId, uint16_t value);

        // Commands from ViewModel - Lights
        void ps_toggleLight(const QString &lightId, bool enabled);
        void ps_updateLightValue(const QString &lightId, uint16_t value);

        void ps_emergencyStop();

    private:
        Unique<FocalTestService> m_focalTestService;
        Unique<LightingTestService> m_lightingTestService;

        Shared<HAL::Act::ActuatorRegistry> m_actuatorRegistry;
        std::vector<std::string> m_knownFocalIds;
        std::vector<std::string> m_knownLightIds;
    };

} // namespace Kub3::Tools::Tester