#if defined(KUB_MODEL_8)
#pragma once

#include <QElapsedTimer>
#include <memory>

#include <Config/conf.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/BaseTaskService.h>
#include <utils.h>

#include "IDrawerService.h"

namespace Kub3::Services
{

    class DualConveyorDrawerService final : public BaseTaskService<IDrawerService>
    {
    public:
        DualConveyorDrawerService(
            Shared<HAL::Act::ActuatorRegistry> registry,
            Shared<HAL::MS::IMachineStatusRepo> repo,
            const Config::process_config_t &processConfig);

        void insert(DrawerTarget target) override;
        void eject(DrawerTarget target) override;
        void stop(void) override;

    private:
        bool isWaferEjected(void);
        bool isMaskInserted(void);
        void loadMotorsKinematicProfiles(void);
        void loadThresholds(void);

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        const Config::process_config_t &m_processConfig;

        // Kinematic profiles
        // --- Wafer
        Config::kinematic_profile_t m_waferFastProfile;
        Config::kinematic_profile_t m_waferFineProfile;
        // --- Mask
        Config::kinematic_profile_t m_maskFastProfile;
        Config::kinematic_profile_t m_maskFineProfile;
        Config::kinematic_profile_t m_maskContactProfile;
        // Threshold values
        int32_t m_waferEjectionFinePosThreshold;
        int32_t m_maskEjectionFinePosThreshold;
    };

}

#endif // KUB_MODEL_8
