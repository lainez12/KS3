#pragma once

#include <Config/conf.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/BaseTaskService.h>
#include <Services/Homing/tasks/Homing/AlignmentStagesHomingTask.h>
#include <Services/Homing/tasks/Homing/CamerasHomingTask.h>
#include <Services/Homing/tasks/Homing/ZMotorsHomingTask.h>

#include "IHomingService.h"

namespace Kub3::Services
{

    class HomingService final : public BaseTaskService<IHomingService>
    {
    public:
        HomingService(
            Shared<HAL::Act::ActuatorRegistry> registry,
            Shared<HAL::MS::IMachineStatusRepo> repo,
            const Config::process_config_t &processConfig);

        void initialize(void) override;
        void home(HomingTarget::Type target = HomingTarget::ALL) override;
        void initializeGranular(HomingTarget::Type target) override;

    protected:
        void onStop(void) override;

    private:
        void stopAllMotors(void);
        void loadMotorsKinematicProfiles(void);

        // Lanes build helpers
        void buildStagesSequence(bool init, uint8_t lane = 0);
        void buildCamerasSequence(bool init, uint8_t lane = 0);
        void buildDeckSequence(bool init, uint8_t lane = 0);

    private: // TODO: Could be general and defined in a `utils` ?
        enum class CameraMotorIdArg
        {
            LeftX,
            LeftY,
            RightX,
            RightY
        };
        Result<homing_cam_bundle_t, std::string> buildCameraHomingMotorBundle(CameraMotorIdArg arg);

        enum class ZMotorIdArg
        {
            Left,
            Right,
            Back,
        };
        Result<z_motor_bundle_t, std::string> buildZMotorBundle(ZMotorIdArg arg);

        enum class StageMotorIdArg
        {
            XStage,
            YStage,
            ThetaStage,
        };
        Result<stage_motor_bundle_t, std::string> buildStageMotorBundle(StageMotorIdArg id);

    private:
        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        const Config::process_config_t &m_processConf;

        // Kinematic profiles
        // --- Z motors
        Config::kinematic_profile_t m_leftFastProfile;
        Config::kinematic_profile_t m_rightFastProfile;
        Config::kinematic_profile_t m_backFastProfile;
        Config::kinematic_profile_t m_leftFineProfile;
        Config::kinematic_profile_t m_rightFineProfile;
        Config::kinematic_profile_t m_backFineProfile;
        // --- Alignment stages
        Config::kinematic_profile_t m_xStageKineProfile;
        Config::kinematic_profile_t m_yStageKineProfile;
        Config::kinematic_profile_t m_thetaStageKineProfile;
        // --- Drawer conveyors
        Config::kinematic_profile_t m_maskConveyorFastProfile;
        Config::kinematic_profile_t m_maskConveyorFineProfile;
        Config::kinematic_profile_t m_maskConveyorContactProfile;
        Config::kinematic_profile_t m_waferConveyorFastProfile;
        Config::kinematic_profile_t m_waferConveyorFineProfile;
        // --- Cameras / Deck
        Config::kinematic_profile_t m_deckKineProfile;
        Config::kinematic_profile_t m_leftCameraXKineProfile;
        Config::kinematic_profile_t m_leftCameraYKineProfile;
        Config::kinematic_profile_t m_rightCameraXKineProfile;
        Config::kinematic_profile_t m_rightCameraYKineProfile;
    };

}
