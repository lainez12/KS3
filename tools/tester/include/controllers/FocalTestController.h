#pragma once

#include <QObject>
#include <QString>
#include <memory>
#include <string>
#include <vector>

#include <HAL/Actuators/ActuatorRegistry.h>
#include <services/FocalTestService.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{

    class FocalTestController : public QObject
    {
        Q_OBJECT

    public:
        explicit FocalTestController(Shared<HAL::Act::ActuatorRegistry> actuatorRegistry,
                                     std::vector<std::string> knownFocalIds,
                                     QObject *parent = nullptr);
        ~FocalTestController() override;

    public slots:
        // System lifecycle
        void start();
        void stop();

        // Commands from ViewModel
        void ps_toggleFocal(const QString &focalId, bool enabled);
        void ps_updateFocalValue(const QString &focalId, uint16_t value);
        void ps_emergencyStop();

    private:
        Unique<FocalTestService> m_focalTestService;

        Shared<HAL::Act::ActuatorRegistry> m_actuatorRegistry;
        std::vector<std::string> m_knownFocalIds;
    };

} // namespace Kub3::Tools::Tester