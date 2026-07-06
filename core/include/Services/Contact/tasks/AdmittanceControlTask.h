#pragma once

#include <QElapsedTimer>
#include <array>
#include <functional>
#include <string>

#include <Algorithms/Admittance/AdmittanceController.h>
#include <Config/kinematics.h>
#include <HAL/Actuators/Motors/IPositionMotor.h>
#include <Services/ITask.h>
#include <utils.h>

// Admittance is the inverse of Impedance
// WEC: Wedge Error Compensation

namespace Kub3::Services
{

    typedef struct force_readings_s {
        double left;
        double right;
        double back;
        double max;
    } force_readings_t;

    class AdmittanceControlTask final : public ITask
    {
    public:
        enum class Mode
        {
            Autoleveling,
            BasicContact
        };

        AdmittanceControlTask(std::array<Shared<HAL::Act::IPositionMotor>, 3> motors,
                              std::function<force_readings_t()> forceGetter,
                              std::function<void(const std::string &)> abortCb,
                              Algorithms::Control::admittance_config_t config,
                              Mode mode,
                              Config::kinematic_profile_t baseProfile);

        void start(void) override;
        bool tick(void) override;

    private:
        std::array<Shared<HAL::Act::IPositionMotor>, 3> m_motors;
        std::function<force_readings_t()> m_forceGetter;
        std::function<void(const std::string &)> m_abortCb;

        Algorithms::Control::admittance_config_t m_config;
        Mode m_mode;
        Config::kinematic_profile_t m_baseProfile;

        QElapsedTimer m_dtTimer;
        bool m_isFirstTick      = true;
        int64_t m_lastTickNsecs = 0;
        QElapsedTimer m_convergenceTimer;
    };

} // namespace Kub3::Services
