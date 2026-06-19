#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/Actuators/Lights/ILightingSystem.h>
#include <Services/IService.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{

    class LightingTestService : public Services::IService
    {
    public:
        LightingTestService(Shared<HAL::Act::ActuatorRegistry> registry,
                            std::vector<std::string> knownLightIds);
        ~LightingTestService() override = default;

        // --- IService Interface ---
        void tick(void) override;
        void stop(void) override;
        [[nodiscard]] Services::ServiceStatus getStatus(void) const noexcept override
        {
            return m_status;
        }
        [[nodiscard]] std::string getErrorReason(void) const override
        {
            return m_errorReason;
        }

        // --- Lights Specific API ---
        void toggleLight(const std::string &lightId, bool enabled);
        void updateLightValue(const std::string &lightId, double value);

        [[nodiscard]] const std::vector<std::string> &getAvailableLights() const;

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        std::vector<std::string> m_knownLightIds;

        Services::ServiceStatus m_status = Services::ServiceStatus::Success; // Idle/Ready
        std::string m_errorReason;
    };

} // namespace Kub3::Tools::Tester
