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

#include <Config/helper.h>

#define CAMERAS_TASKS_QUEUE_LANE 1
#define DECK_TASKS_QUEUE_LANE    2
#define INITIALIZATION_TIMEOUT   90000 // ms (1m30s)

namespace
{
    using namespace Kub3::Services;

    // Local conversion fonction
    init_cam_bundle_t _conv(const homing_cam_bundle_t &input)
    {
        return init_cam_bundle_t{
            .motor            = input.motor,
            .kinematicProfile = input.kinematicProfile,
        };
    }
}

namespace Kub3::Services
{

    HomingService::HomingService(Shared<HAL::Act::ActuatorRegistry> registry,
                                 Shared<HAL::MS::IMachineStatusRepo> repo,
                                 const Config::process_config_t &processConf) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo)),
        m_processConf(processConf)
    {
        loadMotorsKinematicProfiles();
    }

    void HomingService::initialize(void)
    {
        this->clearTasks();

#if defined(KUB_MODEL_8)
        // VISUALIZATION-UNRELATED TASKS LANE BUILD
        {
            const bool z1      = Kub3::HAL::MS::readBool(m_repo, Z1);
            const bool z2      = Kub3::HAL::MS::readBool(m_repo, Z2);
            const bool waferOn = Kub3::HAL::MS::readBool(m_repo, WAFER_ON);
            UNWRAP_OR_ABORT(leftZMotorBundle, buildZMotorBundle(ZMotorIdArg::Left));
            UNWRAP_OR_ABORT(rightZMotorBundle, buildZMotorBundle(ZMotorIdArg::Right));
            UNWRAP_OR_ABORT(backZMotorBundle, buildZMotorBundle(ZMotorIdArg::Back));
            UNWRAP_OR_ABORT(xStageBundle, buildStageMotorBundle(StageMotorIdArg::XStage));
            UNWRAP_OR_ABORT(yStageBundle, buildStageMotorBundle(StageMotorIdArg::YStage));
            UNWRAP_OR_ABORT(thetaStageBundle, buildStageMotorBundle(StageMotorIdArg::ThetaStage));
            UNWRAP_OR_ABORT(maskConvMotor, m_registry->get<HAL::Act::IPositionMotor>(MASK_DRAWER_MOTOR));
            UNWRAP_OR_ABORT(waferConvMotor, m_registry->get<HAL::Act::IPositionMotor>(WAFER_DRAWER_MOTOR));

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

            buildStagesSequence(true);
            // Init 3Z (lower to T2MK low limits)
            enqueueTask<ZMotorsInitTask>(m_repo, leftZMotorBundle, rightZMotorBundle, backZMotorBundle);
            // TODO: Add task to tare force sensors here
            // Init mask conveyor
            enqueueTask<MaskConveyorInitTask>(
                m_repo, maskConvMotor,
                m_maskConveyorFastProfile, m_maskConveyorFineProfile, m_maskConveyorContactProfile,
                m_processConf.drawers.cm3_reset_pos_mm);
            // Init wafer conveyor
            enqueueTask<WaferConveyorInitTask>(m_repo, waferConvMotor, m_waferConveyorFastProfile, m_waferConveyorFineProfile);
        }
        // CAMERAS TASKS LANE BUILD
        buildCamerasSequence(true, CAMERAS_TASKS_QUEUE_LANE);
        // DECK TASKS LANE BUILD
        buildDeckSequence(true, DECK_TASKS_QUEUE_LANE);
#endif

        this->startSequence(INITIALIZATION_TIMEOUT);
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
            UNWRAP_OR_ABORT(leftZMotorBundle, buildZMotorBundle(ZMotorIdArg::Left));
            UNWRAP_OR_ABORT(rightZMotorBundle, buildZMotorBundle(ZMotorIdArg::Right));
            UNWRAP_OR_ABORT(backZMotorBundle, buildZMotorBundle(ZMotorIdArg::Back));
            UNWRAP_OR_ABORT(xStageBundle, buildStageMotorBundle(StageMotorIdArg::XStage));
            UNWRAP_OR_ABORT(yStageBundle, buildStageMotorBundle(StageMotorIdArg::YStage));
            UNWRAP_OR_ABORT(thetaStageBundle, buildStageMotorBundle(StageMotorIdArg::ThetaStage));
            UNWRAP_OR_ABORT(maskConvMotor, m_registry->get<HAL::Act::IPositionMotor>(MASK_DRAWER_MOTOR));
            UNWRAP_OR_ABORT(waferConvMotor, m_registry->get<HAL::Act::IPositionMotor>(WAFER_DRAWER_MOTOR));

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
            UNWRAP_OR_ABORT(leftCamXBundle, buildCameraHomingMotorBundle(CameraMotorIdArg::LeftX));
            UNWRAP_OR_ABORT(leftCamYBundle, buildCameraHomingMotorBundle(CameraMotorIdArg::LeftY));
            UNWRAP_OR_ABORT(rightCamXBundle, buildCameraHomingMotorBundle(CameraMotorIdArg::RightX));
            UNWRAP_OR_ABORT(rightCamYBundle, buildCameraHomingMotorBundle(CameraMotorIdArg::RightY));
            enqueueTask<CamerasHomingTask, CAMERAS_TASKS_QUEUE_LANE>(
                m_repo, leftCamXBundle, leftCamYBundle, rightCamXBundle, rightCamYBundle);
        }

        // Deck homing
        if (target & HomingTarget::DECK)
        {
            UNWRAP_OR_ABORT(deckMotor, m_registry->get<HAL::Act::IMotor>(DECK_MOTOR));
            enqueueTask<DeckHomingTask, DECK_TASKS_QUEUE_LANE>(m_repo, deckMotor, m_deckKineProfile);
        }
#endif

        this->startSequence();
    }

    void HomingService::initializeGranular(HomingTarget::Type target)
    {
        this->clearTasks();

        switch (target)
        {
        case HomingTarget::ALIGNMENT_STAGES:
        {
            buildStagesSequence(true);
            break;
        }
        case HomingTarget::CAMERAS:
        {
            buildCamerasSequence(true);
            break;
        }
        case HomingTarget::DECK:
        {
            buildDeckSequence(true);
            break;
        }
        case HomingTarget::CAMERAS | HomingTarget::DECK:
        {
            buildCamerasSequence(true, CAMERAS_TASKS_QUEUE_LANE);
            buildDeckSequence(true, DECK_TASKS_QUEUE_LANE);
            break;
        }
        default:
            qWarning() << QStringLiteral("[HomingService] Invalid target %1 for method `initializeGranular`. Skipping.").arg(target);
            return; // Invalid target, skipping request.
        }

        this->startSequence(INITIALIZATION_TIMEOUT);
    }

    void HomingService::onStop(void)
    {
        this->stopAllMotors();
    }

    void HomingService::stopAllMotors(void)
    {
        STOP_MOTOR_RESULT(MASK_DRAWER_MOTOR, m_registry);
        STOP_MOTOR_RESULT(WAFER_DRAWER_MOTOR, m_registry);
        STOP_MOTOR_RESULT(Z_LEFT_MOTOR, m_registry);
        STOP_MOTOR_RESULT(Z_RIGHT_MOTOR, m_registry);
        STOP_MOTOR_RESULT(Z_BACK_MOTOR, m_registry);
        STOP_MOTOR_RESULT(X_STAGE_MOTOR, m_registry);
        STOP_MOTOR_RESULT(Y_STAGE_MOTOR, m_registry);
        STOP_MOTOR_RESULT(THETA_STAGE_MOTOR, m_registry);
        STOP_MOTOR_RESULT(LEFT_CAMERA_X_MOTOR, m_registry);
        STOP_MOTOR_RESULT(LEFT_CAMERA_Y_MOTOR, m_registry);
        STOP_MOTOR_RESULT(RIGHT_CAMERA_X_MOTOR, m_registry);
        STOP_MOTOR_RESULT(RIGHT_CAMERA_Y_MOTOR, m_registry);
        STOP_MOTOR_RESULT(DECK_MOTOR, m_registry);
    }

    void HomingService::loadMotorsKinematicProfiles(void)
    {
        // --- Z motors
        m_leftFastProfile  = m_processConf.getKinematicProfile(Z_LEFT_MOTOR, "normal");
        m_leftFineProfile  = m_processConf.getKinematicProfile(Z_LEFT_MOTOR, "fine");
        m_rightFastProfile = m_processConf.getKinematicProfile(Z_RIGHT_MOTOR, "normal");
        m_rightFineProfile = m_processConf.getKinematicProfile(Z_RIGHT_MOTOR, "fine");
        m_backFastProfile  = m_processConf.getKinematicProfile(Z_BACK_MOTOR, "normal");
        m_backFineProfile  = m_processConf.getKinematicProfile(Z_BACK_MOTOR, "fine");
        // --- Alignment Stages
        m_xStageKineProfile     = m_processConf.getKinematicProfile(X_STAGE_MOTOR, "normal");
        m_yStageKineProfile     = m_processConf.getKinematicProfile(Y_STAGE_MOTOR, "normal");
        m_thetaStageKineProfile = m_processConf.getKinematicProfile(THETA_STAGE_MOTOR, "normal");
        // --- Drawer conveyors
        m_maskConveyorFastProfile    = m_processConf.getKinematicProfile(MASK_DRAWER_MOTOR, "normal");
        m_maskConveyorFineProfile    = m_processConf.getKinematicProfile(MASK_DRAWER_MOTOR, "fine");
        m_maskConveyorContactProfile = m_processConf.getKinematicProfile(MASK_DRAWER_MOTOR, "contact");
        m_waferConveyorFastProfile   = m_processConf.getKinematicProfile(WAFER_DRAWER_MOTOR, "normal");
        m_waferConveyorFineProfile   = m_processConf.getKinematicProfile(WAFER_DRAWER_MOTOR, "fine");
        // --- Deck / cameras
        m_deckKineProfile         = m_processConf.getKinematicProfile(DECK_MOTOR, "normal");
        m_leftCameraXKineProfile  = m_processConf.getKinematicProfile(LEFT_CAMERA_X_MOTOR, "normal");
        m_leftCameraYKineProfile  = m_processConf.getKinematicProfile(LEFT_CAMERA_Y_MOTOR, "normal");
        m_rightCameraXKineProfile = m_processConf.getKinematicProfile(RIGHT_CAMERA_X_MOTOR, "normal");
        m_rightCameraYKineProfile = m_processConf.getKinematicProfile(RIGHT_CAMERA_Y_MOTOR, "normal");
    }

    void HomingService::buildStagesSequence(bool init, uint8_t lane)
    {
        UNWRAP_OR_ABORT(xStageBundle, buildStageMotorBundle(StageMotorIdArg::XStage));
        UNWRAP_OR_ABORT(yStageBundle, buildStageMotorBundle(StageMotorIdArg::YStage));
        UNWRAP_OR_ABORT(thetaStageBundle, buildStageMotorBundle(StageMotorIdArg::ThetaStage));

        if (init)
        {
            enqueueTaskOnLane<AlignmentStagesInitTask>(
                lane, m_repo,
                xStageBundle.motor, yStageBundle.motor, thetaStageBundle.motor,
                xStageBundle.kinematic, yStageBundle.kinematic, thetaStageBundle.kinematic);
        }

        enqueueTaskOnLane<AlignmentStagesHomingTask>(lane, m_repo, xStageBundle, yStageBundle, thetaStageBundle);
    }

    void HomingService::buildCamerasSequence(bool init, uint8_t lane)
    {
        UNWRAP_OR_ABORT(leftCamXBundle, buildCameraHomingMotorBundle(CameraMotorIdArg::LeftX));
        UNWRAP_OR_ABORT(leftCamYBundle, buildCameraHomingMotorBundle(CameraMotorIdArg::LeftY));
        UNWRAP_OR_ABORT(rightCamXBundle, buildCameraHomingMotorBundle(CameraMotorIdArg::RightX));
        UNWRAP_OR_ABORT(rightCamYBundle, buildCameraHomingMotorBundle(CameraMotorIdArg::RightY));

        if (init)
        {
            enqueueTaskOnLane<CamerasInitTask>(
                lane, m_repo, m_processConf,
                _conv(leftCamXBundle), _conv(leftCamYBundle), _conv(rightCamXBundle), _conv(rightCamYBundle));
        }
        enqueueTaskOnLane<CamerasHomingTask>(
            lane, m_repo,
            leftCamXBundle, leftCamYBundle, rightCamXBundle, rightCamYBundle);
    }

    void HomingService::buildDeckSequence(bool init, uint8_t lane)
    {
        UNWRAP_OR_ABORT(deckMotor, m_registry->get<HAL::Act::IMotor>(DECK_MOTOR));

        if (init)
            enqueueTaskOnLane<DeckInitTask>(lane, m_repo, deckMotor, m_deckKineProfile);
        else
            enqueueTaskOnLane<DeckHomingTask>(lane, m_repo, deckMotor, m_deckKineProfile);
    }

    Result<homing_cam_bundle_t, std::string> HomingService::buildCameraHomingMotorBundle(CameraMotorIdArg arg)
    {
        const char *motorId = nullptr;
        double centerPosMm  = 0.0;
        Config::kinematic_profile_t profile;

        switch (arg)
        {
        case CameraMotorIdArg::LeftX:
            motorId     = LEFT_CAMERA_X_MOTOR;
            centerPosMm = m_processConf.vision.left_cam_x_home_pos_mm;
            profile     = m_leftCameraXKineProfile;
            break;
        case CameraMotorIdArg::LeftY:
            motorId     = LEFT_CAMERA_Y_MOTOR;
            centerPosMm = m_processConf.vision.left_cam_y_home_pos_mm;
            profile     = m_leftCameraYKineProfile;
            break;
        case CameraMotorIdArg::RightX:
            motorId     = RIGHT_CAMERA_X_MOTOR;
            centerPosMm = m_processConf.vision.right_cam_x_home_pos_mm;
            profile     = m_rightCameraXKineProfile;
            break;
        case CameraMotorIdArg::RightY:
            motorId     = RIGHT_CAMERA_Y_MOTOR;
            centerPosMm = m_processConf.vision.right_cam_y_home_pos_mm;
            profile     = m_rightCameraYKineProfile;
            break;
        }

        auto buildBundle = [=](auto motor) {
            return homing_cam_bundle_t{
                .motor            = motor,
                .centerPositionMm = centerPosMm,
                .kinematicProfile = profile,
            };
        };

        return m_registry->get<HAL::Act::IPositionMotor>(motorId).map(buildBundle);
    }

    Result<z_motor_bundle_t, std::string> HomingService::buildZMotorBundle(ZMotorIdArg arg)
    {
        const char *label                       = nullptr;
        const Config::kinematic_profile_t *fast = nullptr;
        const Config::kinematic_profile_t *fine = nullptr;

        switch (arg)
        {
        case ZMotorIdArg::Left:
            label = Z_LEFT_MOTOR;
            fast  = &m_leftFastProfile;
            fine  = &m_leftFineProfile;
            break;
        case ZMotorIdArg::Right:
            label = Z_RIGHT_MOTOR;
            fast  = &m_rightFastProfile;
            fine  = &m_rightFineProfile;
            break;
        case ZMotorIdArg::Back:
        default:
            label = Z_BACK_MOTOR;
            fast  = &m_backFastProfile;
            fine  = &m_backFineProfile;
            break;
        }

        auto bundleBuilder = [=](auto motor) {
            return z_motor_bundle_t{
                .motor       = motor,
                .fastProfile = *fast,
                .fineProfile = *fine,
            };
        };

        return m_registry->get<HAL::Act::IPositionMotor>(label).map(bundleBuilder);
    }

    Result<stage_motor_bundle_t, std::string> HomingService::buildStageMotorBundle(StageMotorIdArg arg)
    {
        const char *motorId                          = nullptr;
        double centerPositionMm                      = 0.0;
        const Config::kinematic_profile_t *kinematic = nullptr;

        switch (arg)
        {
        case StageMotorIdArg::XStage:
            motorId          = X_STAGE_MOTOR;
            kinematic        = &m_xStageKineProfile;
            centerPositionMm = m_processConf.alignment.x_stage_center_pos_mm;
            break;
        case StageMotorIdArg::YStage:
            motorId          = Y_STAGE_MOTOR;
            kinematic        = &m_yStageKineProfile;
            centerPositionMm = m_processConf.alignment.y_stage_center_pos_mm;
            break;
        case StageMotorIdArg::ThetaStage:
        default:
            motorId          = THETA_STAGE_MOTOR;
            kinematic        = &m_thetaStageKineProfile;
            centerPositionMm = m_processConf.alignment.theta_stage_center_pos_mm;
            break;
        }

        auto bundleBuilder = [=](auto motor) {
            return stage_motor_bundle_t{
                .motor            = motor,
                .kinematic        = *kinematic,
                .centerPositionMm = centerPositionMm,
            };
        };

        return m_registry->get<HAL::Act::IPositionMotor>(motorId).map(bundleBuilder);
    }

}
