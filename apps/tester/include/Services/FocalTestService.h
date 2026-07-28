#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <HAL/Actuators/ActuatorRegistry.h>
#include <Services/IService.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{

    class FocalTestService : public Services::IService
    {
    public:
        explicit FocalTestService(Shared<HAL::Act::ActuatorRegistry> registry,
                                  std::vector<std::string> knownFocalIds);
        ~FocalTestService() override = default;

        // --- IService Interface ---
        void setLogCallback(LogCallback cb) override { m_logCallback = std::move(cb); };
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

        // --- Focal Specific API ---
        void toggleFocal(const std::string &focalId, bool enabled);
        void updateFocalValue(const std::string &focalId, double value);

        [[nodiscard]] const std::vector<std::string> &getAvailableFocals() const;

    private:
        LogCallback m_logCallback;
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        std::vector<std::string> m_knownFocalIds;

        Services::ServiceStatus m_status = Services::ServiceStatus::Success; // Idle/Ready
        std::string m_errorReason;
    };

} // namespace Kub3::Tools::Tester
