#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <string>
#include <vector>

#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Vision/identifiers.h>
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

    signals:
        void s_cameraUpdate(const QString &camId, HAL::Vision::CameraParamKind kind, HAL::Vision::CameraParam value);

    public slots:
        // System lifecycle
        void start();
        void stop();

        // Commands from ViewModel - Cameras
        void ps_requestExposureUpdate(const QString &camId, double exposureRatio);
        void ps_requestGainUpdate(const QString &camId, double gainRatio);
        void ps_requestFrameRateUpdate(const QString &camId, double framerate);
        void ps_requestCenteredZoomUpdate(const QString &camId, double zoomFactor);
        void ps_requestROIUpdate(const QString &camId, const QRect &roi);
        // Commands from ViewModel - Focals
        void ps_toggleFocal(const QString &focalId, bool enabled);
        void ps_updateFocalValue(const QString &focalId, double value);
        // Commands from ViewModel - Lights
        void ps_toggleLight(const QString &lightId, bool enabled);
        void ps_updateLightValue(const QString &lightId, double value);

        void ps_emergencyStop();

    private:
        Unique<FocalTestService> m_focalTestService;
        Unique<LightingTestService> m_lightingTestService;

        Shared<HAL::Act::ActuatorRegistry> m_actuatorRegistry;
        std::vector<std::string> m_knownFocalIds;
        std::vector<std::string> m_knownLightIds;
    };

} // namespace Kub3::Tools::Tester