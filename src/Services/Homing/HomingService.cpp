// HAL
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
// Services & Tasks
#include <Services/Homing/HomingService.h>
#include <Services/Homing/tasks/ZMotorsReachLimitTask.h>
// --- Initialization tasks
#include <Services/Homing/tasks/Initialization/AlignmentStagesInitTask.h>
#include <Services/Homing/tasks/Initialization/CamerasInitTask.h>
#include <Services/Homing/tasks/Initialization/DeckInitTask.h>
#include <Services/Homing/tasks/Initialization/MaskConveyorInitTask.h>
#include <Services/Homing/tasks/Initialization/WaferConveyorInitTask.h>
#include <Services/Homing/tasks/Initialization/ZMotorsInitTask.h>
// --- Homing tasks
#include <Services/Homing/tasks/Homing/AlignmentStagesHomingTask.h>
#include <Services/Homing/tasks/Homing/CamerasHomingTask.h>
#include <Services/Homing/tasks/Homing/DeckHomingTask.h>
#include <Services/Homing/tasks/Homing/MaskHomingTask.h>
#include <Services/Homing/tasks/Homing/WaferHomingTask.h>
#include <Services/Homing/tasks/Homing/ZMotorsHomingTask.h>

#define CAMERAS_TASKS_QUEUE_LANE 1
#define DECK_TASKS_QUEUE_LANE    2

namespace Kub3::Services
{

    HomingService::HomingService(Shared<HAL::Act::ActuatorRegistry> registry,
                                 Shared<HAL::MS::IMachineStatusRepo> repo,
                                 const Config::process_config_t &processConfig) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo)),
        m_processConfig(processConfig)
    {
    }

    void HomingService::initialize(void)
    {
        this->clearTasks();

#if defined(KUB_MODEL_8)
        // VISUALIZATION-UNRELATED TASKS LANE BUILD
        {
            const bool z1                               = Kub3::HAL::MS::readBool(m_repo, Z1);
            const bool z2                               = Kub3::HAL::MS::readBool(m_repo, Z2);
            const bool waferOn                          = Kub3::HAL::MS::readBool(m_repo, WAFER_ON);
            const z_motor_bundle_t leftZMotorBundle     = buildZMotorBundle(ZMotorIdArg::Left);
            const z_motor_bundle_t rightZMotorBundle    = buildZMotorBundle(ZMotorIdArg::Right);
            const z_motor_bundle_t backZMotorBundle     = buildZMotorBundle(ZMotorIdArg::Back);
            const stage_motor_bundle_t xStageBundle     = buildStageMotorBundle(StageMotorIdArg::XStage);
            const stage_motor_bundle_t yStageBundle     = buildStageMotorBundle(StageMotorIdArg::YStage);
            const stage_motor_bundle_t thetaStageBundle = buildStageMotorBundle(StageMotorIdArg::ThetaStage);
            Shared<HAL::Act::IMotor> maskConvMotor      = m_registry->get<HAL::Act::IMotor>(MASK_DRAWER_MOTOR);
            Shared<HAL::Act::IMotor> waferConvMotor     = m_registry->get<HAL::Act::IMotor>(WAFER_DRAWER_MOTOR);

            if (waferOn)
            {
                if (z2)
                {
                    // Lower to unmeet z2 (ensure not in contact with mask)
                    enqueueTask<ZMotorsReachLimitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle, Z2, false);
                }
                // Climb to reach z2 (reach safe zone)
                enqueueTask<ZMotorsReachLimitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle, Z2, true);
            }
            else if (z1)
            {
                // Lower to unmeet z1 (Enter sub-conveyor (wafer) zone)
                enqueueTask<ZMotorsReachLimitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle, Z2, false);
            }

            // Init X, Y, Theta
            enqueueTask<AlignmentStagesInitTask>(
                m_repo,
                xStageBundle.motor, yStageBundle.motor, thetaStageBundle.motor,
                xStageBundle.kinematic, yStageBundle.kinematic, thetaStageBundle.kinematic);
            // Center X, Y, Theta
            enqueueTask<AlignmentStagesHomingTask>(m_repo, xStageBundle, yStageBundle, thetaStageBundle);
            // Init 3Z (lower to T2MK low limits)
            enqueueTask<ZMotorsInitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle);
            // TODO: Tare force sensors here
            // Init mask conveyor
            enqueueTask<MaskConveyorInitTask>(
                m_repo, maskConvMotor,
                m_maskConveyorFastProfile, m_maskConveyorFineProfile, m_maskConveyorContactProfile,
                m_processConfig.cm3_reset_pos_mm);
            // Init wafer conveyor
            enqueueTask<WaferConveyorInitTask>(m_repo, waferConvMotor, m_waferConveyorFastProfile, m_waferConveyorFineProfile);
        }
        // CAMERAS TASKS LANE BUILD
        {
            const camera_motor_bundle_t leftCamXBundle  = buildCameraMotorBundle(CameraMotorIdArg::LeftX);
            const camera_motor_bundle_t leftCamYBundle  = buildCameraMotorBundle(CameraMotorIdArg::LeftY);
            const camera_motor_bundle_t rightCamXBundle = buildCameraMotorBundle(CameraMotorIdArg::RightX);
            const camera_motor_bundle_t rightCamYBundle = buildCameraMotorBundle(CameraMotorIdArg::RightY);

            enqueueTask<CamerasInitTask, CAMERAS_TASKS_QUEUE_LANE>(
                m_repo, m_processConfig,
                leftCamXBundle.motor, leftCamYBundle.motor, rightCamXBundle.motor, rightCamYBundle.motor,
                m_leftCameraXKineProfile);
            enqueueTask<CamerasHomingTask, CAMERAS_TASKS_QUEUE_LANE>(
                m_repo, leftCamXBundle, leftCamYBundle, rightCamXBundle, rightCamYBundle,
                m_leftCameraXKineProfile);
        }
        // DECK TASKS LANE BUILD
        {
            Shared<HAL::Act::IMotor> deckMotor = m_registry->get<HAL::Act::IMotor>(DECK_MOTOR);

            enqueueTask<DeckInitTask, DECK_TASKS_QUEUE_LANE>(m_repo, deckMotor, m_deckKineProfile);
            enqueueTask<DeckHomingTask, DECK_TASKS_QUEUE_LANE>(m_repo, deckMotor, m_deckKineProfile);
        }
#endif

        this->startSequence();
    }

    void HomingService::home(HomingTarget::Type target)
    {
        this->clearTasks();

#if defined(KUB_MODEL_8)
        // Populate Alignments stages, 3Z, conveyors movements
        {
            // Sensors
            const bool z1      = Kub3::HAL::MS::readBool(m_repo, Z1);
            const bool z2      = Kub3::HAL::MS::readBool(m_repo, Z2);
            const bool waferOn = Kub3::HAL::MS::readBool(m_repo, WAFER_ON);
            const bool cw2     = Kub3::HAL::MS::readBool(m_repo, CW2);
            const bool cm2     = Kub3::HAL::MS::readBool(m_repo, CM2);
            const bool cm3     = Kub3::HAL::MS::readBool(m_repo, CM3);

            // Actuators
            const z_motor_bundle_t leftZMotorBundle     = buildZMotorBundle(ZMotorIdArg::Left);
            const z_motor_bundle_t rightZMotorBundle    = buildZMotorBundle(ZMotorIdArg::Right);
            const z_motor_bundle_t backZMotorBundle     = buildZMotorBundle(ZMotorIdArg::Back);
            const stage_motor_bundle_t xStageBundle     = buildStageMotorBundle(StageMotorIdArg::XStage);
            const stage_motor_bundle_t yStageBundle     = buildStageMotorBundle(StageMotorIdArg::YStage);
            const stage_motor_bundle_t thetaStageBundle = buildStageMotorBundle(StageMotorIdArg::ThetaStage);
            Shared<HAL::Act::IMotor> maskConvMotor      = m_registry->get<HAL::Act::IMotor>(MASK_DRAWER_MOTOR);
            Shared<HAL::Act::IMotor> waferConvMotor     = m_registry->get<HAL::Act::IMotor>(WAFER_DRAWER_MOTOR);

            // Requests
            const bool alignmentHomeRequested     = (target & HomingTarget::ALIGNMENT_STAGES); // Needs !Z1 or contact Z2
            const bool zMotorsHomeRequested       = (target & HomingTarget::Z_MOTORS);         // Needs CW2
            const bool maskConveyorHomeRequested  = (target & HomingTarget::MASK_CONVEYOR);    // Needs !Z2
            const bool waferConveyorHomeRequested = (target & HomingTarget::WAFER_CONVEYOR);   // Needs CM2 || CM3

            // Virtual state tracking variables for the pipeline
            bool curr_z1 = z1;
            bool curr_z2 = z2;

            // 1. MASK CONVEYOR (Requires !Z2)
            if (maskConveyorHomeRequested)
            {
                if (curr_z2 && !zMotorsHomeRequested)
                {
                    this->abortSequence("The mask conveyor homing cannot be performed without permission to move Z motors.");
                    return;
                }

                if (curr_z2)
                {
                    // Safely unmeet Z2 to clear the way for mask
                    enqueueTask<ZMotorsReachLimitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle, Z2, false);
                    curr_z2 = false;
                }

                enqueueTask<MaskHomingTask>(
                    m_repo, maskConvMotor,
                    m_maskConveyorFastProfile, m_maskConveyorFineProfile, m_maskConveyorContactProfile);
            }

            // 2. ALIGNMENT STAGES (Must be handled BEFORE Wafer Conveyor drops Z)
            if (alignmentHomeRequested)
            {
                if (curr_z1 && !zMotorsHomeRequested)
                {
                    this->abortSequence("Alignment stages centering cannot be performed without permission to move Z motors.");
                    return;
                }

                // If wafer is present OR we physically started at Z2, traversing down is forbidden. We ALWAYS move up to Z2 to center
                if (waferOn || z2)
                {
                    if (curr_z2)
                        enqueueTask<ZMotorsReachLimitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle, Z2, false);
                    enqueueTask<ZMotorsReachLimitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle, Z2, true);
                    curr_z2 = true;
                    curr_z1 = true;
                }
                else // waferOn == false AND z2 == false
                {
                    if (curr_z1) // Safe to go down to !Z1 to center
                    {
                        enqueueTask<ZMotorsReachLimitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle, Z1, false);
                        curr_z1 = false;
                        curr_z2 = false;
                    }
                }

                enqueueTask<AlignmentStagesHomingTask>(m_repo, xStageBundle, yStageBundle, thetaStageBundle);
            }

            // 3. WAFER CONVEYOR (Needs Mask inserted and !Z1)
            if (waferConveyorHomeRequested)
            {
                if ((curr_z1 && !zMotorsHomeRequested) || (!cm2 && !cm3 && !maskConveyorHomeRequested))
                {
                    if (curr_z1)
                        this->abortSequence("The wafer conveyor homing cannot be performed without permission to move Z motors.");
                    else
                        this->abortSequence("The wafer conveyor homing cannot be performed without permission to move the mask conveyor.");
                    return;
                }

                if (curr_z1)
                {
                    // Check Danger Zone traversal: If wafer is ON or we started at Z2, dropping below Z1 traverses the danger zone.
                    // This is ONLY legal if we centered
                    if ((waferOn || z2) && !alignmentHomeRequested)
                    {
                        this->abortSequence("Cannot lower Z motors for wafer conveyor homing as traversing the stowage zone requires centering the alignment stages.");
                        return;
                    }

                    // Lower Z below Z1 before attempting to insert the wafer conveyor.
                    enqueueTask<ZMotorsReachLimitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle, Z1, false);
                    curr_z1 = false;
                    curr_z2 = false;
                }

                enqueueTask<WaferHomingTask>(m_repo, waferConvMotor, m_waferConveyorFastProfile, m_waferConveyorFineProfile);
            }

            // 4. Z MOTORS (Needs Wafer inserted -> cw2)
            if (zMotorsHomeRequested)
            {
                if (!cw2 && !waferConveyorHomeRequested)
                {
                    this->abortSequence("Z motors homing cannot be performed without permission to move the wafer conveyor.");
                    return;
                }

                enqueueTask<ZMotorsHomingTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle);
            }
        }

        // Cameras homing
        if (target & HomingTarget::CAMERAS)
        {
            const camera_motor_bundle_t leftCamXBundle  = buildCameraMotorBundle(CameraMotorIdArg::LeftX);
            const camera_motor_bundle_t leftCamYBundle  = buildCameraMotorBundle(CameraMotorIdArg::LeftY);
            const camera_motor_bundle_t rightCamXBundle = buildCameraMotorBundle(CameraMotorIdArg::RightX);
            const camera_motor_bundle_t rightCamYBundle = buildCameraMotorBundle(CameraMotorIdArg::RightY);

            enqueueTask<CamerasHomingTask, CAMERAS_TASKS_QUEUE_LANE>(
                m_repo, leftCamXBundle, leftCamYBundle, rightCamXBundle, rightCamYBundle, m_leftCameraXKineProfile);
        }

        // Deck homing
        if (target & HomingTarget::DECK)
            enqueueTask<DeckHomingTask, DECK_TASKS_QUEUE_LANE>(m_repo, m_registry->get<HAL::Act::IMotor>(DECK_MOTOR), m_deckKineProfile);
#endif

        this->startSequence();
    }

    void HomingService::stop(void)
    {
        BaseTaskService::stop();
    }

    void HomingService::loadMotorsKinematicProfiles(void)
    {
        // --- Z motors
        m_leftFastProfile  = m_processConfig.getKinematicProfile(Z_LEFT_MOTOR, "normal");
        m_leftFineProfile  = m_processConfig.getKinematicProfile(Z_LEFT_MOTOR, "fine");
        m_rightFastProfile = m_processConfig.getKinematicProfile(Z_RIGHT_MOTOR, "normal");
        m_rightFineProfile = m_processConfig.getKinematicProfile(Z_RIGHT_MOTOR, "fine");
        m_backFastProfile  = m_processConfig.getKinematicProfile(Z_BACK_MOTOR, "normal");
        m_backFineProfile  = m_processConfig.getKinematicProfile(Z_BACK_MOTOR, "fine");
        // --- Alignment Stages
        m_xStageKineProfile     = m_processConfig.getKinematicProfile(X_STAGE_MOTOR, "normal");
        m_yStageKineProfile     = m_processConfig.getKinematicProfile(Y_STAGE_MOTOR, "normal");
        m_thetaStageKineProfile = m_processConfig.getKinematicProfile(THETA_STAGE_MOTOR, "normal");
        // --- Drawer conveyors
        m_maskConveyorFastProfile    = m_processConfig.getKinematicProfile(MASK_DRAWER_MOTOR, "normal");
        m_maskConveyorFineProfile    = m_processConfig.getKinematicProfile(MASK_DRAWER_MOTOR, "fine");
        m_maskConveyorContactProfile = m_processConfig.getKinematicProfile(MASK_DRAWER_MOTOR, "contact");
        m_waferConveyorFastProfile   = m_processConfig.getKinematicProfile(WAFER_DRAWER_MOTOR, "normal");
        m_waferConveyorFineProfile   = m_processConfig.getKinematicProfile(WAFER_DRAWER_MOTOR, "fine");
        // --- Deck / cameras
        m_deckKineProfile         = m_processConfig.getKinematicProfile(DECK_MOTOR, "normal");
        m_leftCameraXKineProfile  = m_processConfig.getKinematicProfile(LEFT_CAMERA_X_MOTOR, "normal");
        m_leftCameraYKineProfile  = m_processConfig.getKinematicProfile(LEFT_CAMERA_Y_MOTOR, "normal");
        m_rightCameraXKineProfile = m_processConfig.getKinematicProfile(RIGHT_CAMERA_X_MOTOR, "normal");
        m_rightCameraYKineProfile = m_processConfig.getKinematicProfile(RIGHT_CAMERA_Y_MOTOR, "normal");
    }

    camera_motor_bundle_t HomingService::buildCameraMotorBundle(CameraMotorIdArg arg)
    {
        const char *motorId = nullptr;
        double centerPosMm  = 0.0;

        switch (arg)
        {
        case CameraMotorIdArg::LeftX:
            motorId     = LEFT_CAMERA_X_MOTOR;
            centerPosMm = m_processConfig.left_cam_x_reset_pos_mm;
            break;
        case CameraMotorIdArg::LeftY:
            motorId     = LEFT_CAMERA_Y_MOTOR;
            centerPosMm = m_processConfig.left_cam_y_reset_pos_mm;
            break;
        case CameraMotorIdArg::RightX:
            motorId     = RIGHT_CAMERA_X_MOTOR;
            centerPosMm = m_processConfig.right_cam_x_reset_pos_mm;
            break;
        case CameraMotorIdArg::RightY:
            motorId     = RIGHT_CAMERA_Y_MOTOR;
            centerPosMm = m_processConfig.right_cam_y_reset_pos_mm;
            break;
        }

        return camera_motor_bundle_t{
            .motor            = m_registry->get<HAL::Act::IMotor>(motorId),
            .centerPositionMm = centerPosMm,
        };
    }

    z_motor_bundle_t HomingService::buildZMotorBundle(ZMotorIdArg arg)
    {
        if (arg == ZMotorIdArg::Left)
            return z_motor_bundle_t{
                .motor       = m_registry->get<HAL::Act::IMotor>(Z_LEFT_MOTOR),
                .fastProfile = m_leftFastProfile,
                .fineProfile = m_leftFineProfile,
            };

        if (arg == ZMotorIdArg::Right)
            return z_motor_bundle_t{
                .motor       = m_registry->get<HAL::Act::IMotor>(Z_RIGHT_MOTOR),
                .fastProfile = m_rightFastProfile,
                .fineProfile = m_rightFineProfile,
            };

        // if (arg == ZMotorIdArg::Back)
        return z_motor_bundle_t{
            .motor       = m_registry->get<HAL::Act::IMotor>(Z_BACK_MOTOR),
            .fastProfile = m_backFastProfile,
            .fineProfile = m_backFineProfile,
        };
    }

    stage_motor_bundle_t HomingService::buildStageMotorBundle(StageMotorIdArg arg)
    {
        if (arg == StageMotorIdArg::XStage)
            return stage_motor_bundle_t{
                .motor            = m_registry->get<HAL::Act::IMotor>(X_STAGE_MOTOR),
                .kinematic        = m_xStageKineProfile,
                .centerPositionMm = m_processConfig.x_stage_center_pos_mm};

        if (arg == StageMotorIdArg::YStage)
            return stage_motor_bundle_t{
                .motor            = m_registry->get<HAL::Act::IMotor>(Y_STAGE_MOTOR),
                .kinematic        = m_yStageKineProfile,
                .centerPositionMm = m_processConfig.y_stage_center_pos_mm,
            };

        // if (arg == StageMotorIdArg::ThetaStage)
        return stage_motor_bundle_t{
            .motor            = m_registry->get<HAL::Act::IMotor>(THETA_STAGE_MOTOR),
            .kinematic        = m_thetaStageKineProfile,
            .centerPositionMm = m_processConfig.theta_stage_center_pos_mm,
        };
    }

}
