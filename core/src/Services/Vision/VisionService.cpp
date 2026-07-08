#include <QDebug>

#include <HAL/Actuators/Focal/IFocal.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Vision/VisionService.h>

namespace
{
    using Kub3::Services::VisionMotor;

    inline std::ostream &operator<<(std::ostream &os, const VisionMotor &motorId)
    {
        switch (motorId)
        {
        case VisionMotor::UpperLeftCameraX:
            return os << "UpperLeftCameraX";
        case VisionMotor::UpperLeftCameraY:
            return os << "UpperLeftCameraY";
        case VisionMotor::UpperRightCameraX:
            return os << "UpperRightCameraX";
        case VisionMotor::UpperRightCameraY:
            return os << "UpperRightCameraY";
        default:
            break;
        }
        return os;
    }

    inline QDebug operator<<(QDebug dbg, const VisionMotor &motorId)
    {
        switch (motorId)
        {
        case VisionMotor::UpperLeftCameraX:
            return dbg << "UpperLeftCameraX";
        case VisionMotor::UpperLeftCameraY:
            return dbg << "UpperLeftCameraY";
        case VisionMotor::UpperRightCameraX:
            return dbg << "UpperRightCameraX";
        case VisionMotor::UpperRightCameraY:
            return dbg << "UpperRightCameraY";
        default:
            break;
        }
        return dbg;
    }

}

namespace Kub3::Services
{
    VisionService::VisionService(Shared<HAL::Act::ActuatorRegistry> registry,
                                 Shared<HAL::MS::IMachineStatusRepo> repo,
                                 const Config::process_config_t &processConf) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo)),
        m_conf(processConf.vision)
    {
        setupCameraMotor(VisionMotor::UpperLeftCameraX, LEFT_CAMERA_X_MOTOR, processConf);
        setupCameraMotor(VisionMotor::UpperLeftCameraY, LEFT_CAMERA_Y_MOTOR, processConf);
        setupCameraMotor(VisionMotor::UpperRightCameraX, RIGHT_CAMERA_X_MOTOR, processConf);
        setupCameraMotor(VisionMotor::UpperRightCameraY, RIGHT_CAMERA_Y_MOTOR, processConf);
        UNWRAP_OR_THROW(leftFocal, m_registry->get<HAL::Act::IFocal>(LEFT_CAMERA_FOCAL), "[VisionService] Failed to load upper left camera focal: ");
        UNWRAP_OR_THROW(rightFocal, m_registry->get<HAL::Act::IFocal>(RIGHT_CAMERA_FOCAL), "[VisionService] Failed to load upper right camera focal: ");
        UNWRAP_OR_THROW(deckMotorRes, m_registry->get<HAL::Act::IMotor>(DECK_MOTOR), "[VisionService] Failed to load Camera's Deck Motor: ");

        // Load camera's deck motor & config
        m_deckMotor   = deckMotorRes;
        m_deckProfile = processConf.getKinematicProfile(DECK_MOTOR, "normal");
        // Load focal configurations
        m_focalConfs.emplace(LEFT_CAMERA_FOCAL, m_conf.left_focal_conf);
        m_focalConfs.emplace(RIGHT_CAMERA_FOCAL, m_conf.right_focal_conf);
        // Set focals default values
        setFocalValue(LEFT_CAMERA_FOCAL, m_conf.left_focal_conf.default_value);
        setFocalValue(RIGHT_CAMERA_FOCAL, m_conf.right_focal_conf.default_value);
    }

    void VisionService::setupCameraMotor(VisionMotor motorId, const char *motorConfId, const Config::process_config_t &conf)
    {
        auto motor                     = m_registry->get<HAL::Act::IPositionMotor>(motorConfId);
        auto extractGranularMovementMm = [&conf](VisionMotor id) -> double {
            switch (id)
            {
            case VisionMotor::UpperLeftCameraX:
                return conf.pad.left_cam_x_distance_mm;
            case VisionMotor::UpperLeftCameraY:
                return conf.pad.left_cam_y_distance_mm;
            case VisionMotor::UpperRightCameraX:
                return conf.pad.right_cam_x_distance_mm;
            case VisionMotor::UpperRightCameraY:
                return conf.pad.right_cam_y_distance_mm;
            default:
                break;
            }
            return 0.0;
        };

        if (!motor)
        {
            auto err = std::string("[VisionService] Failed to load motor ") + motorConfId + ": " + motor.unwrap_err();

            qCritical().noquote() << err;
            throw std::runtime_error(err);
        }

        m_cameraMotors.emplace(
            motorId,
            vision_motor_config_t{
                .motor              = motor.unwrap(),
                .fastProfile        = conf.getKinematicProfile(motorConfId, "normal"),
                .fineProfile        = conf.getKinematicProfile(motorConfId, "fine"),
                .granularMovementMm = extractGranularMovementMm(motorId),
            });
    }

    void VisionService::tick(void)
    {
        // Deck Sequence Automation Loop
        if (m_status == ServiceStatus::Running && m_isDeckMoving)
        {
            if (m_deckMotor && deckVisualisationLimitReached()) // Limit reached
            {
                qInfo() << "VisionService: Deck reached visualization position (front).";
                m_deckMotor->emergencyStop();
                m_isDeckMoving = false;
                m_status       = ServiceStatus::Success;
            }
        }

        // Continuous Camera Motor Safety Watchdog Loop
        for (auto &[motorId, config] : m_cameraMotors)
        {
            if (config.watchdogTicks > 0)
            {
                // SAFETY: Continuous anti-collision check while moving
                if (inCollisionZone(motorId, config.currentDir))
                {
                    if (m_pushingModeEnabled)
                        applyPush(motorId, config.fineMode); // PUSHING MODE: Feed the pushed camera's watchdog
                    else
                    {
                        qCritical() << "VisionService: Collision boundary reached. Halting motor.";
                        stopManual(motorId);
                        continue;
                    }
                }

                config.watchdogTicks--;
                if (config.watchdogTicks == 0)
                {
                    qWarning() << "VisionService: Dead-Man's switch triggered for motor:" << motorId;
                    if (config.motor)
                        config.motor->emergencyStop();
                }
            }
        }
    }

    void VisionService::stop(void)
    {
        // Stop all manual camera movements
        for (auto &[motorId, config] : m_cameraMotors)
        {
            config.watchdogTicks = 0;
            if (config.motor)
                config.motor->emergencyStop();
        }

        // Stop automated deck movement
        if (m_deckMotor)
        {
            m_deckMotor->emergencyStop();
        }
        m_isDeckMoving = false;
        m_errorReason.clear();
        m_status = ServiceStatus::Idle;
    }

    void VisionService::moveBlockToVisualisationPosition(void)
    {
        if (!m_deckMotor)
        {
            m_errorReason = "Deck motor is not configured or missing from registry.";
            m_status      = ServiceStatus::Error;
            return;
        }

        m_errorReason.clear();

        if (deckVisualisationLimitReached())
        {
            qInfo() << "VisionService: Deck is already at visualization position.";
            m_status       = ServiceStatus::Success; // MUST tell the FSM we are done
            m_isDeckMoving = false;
            return;
        }

        qInfo() << "VisionService: Moving deck to visualization position (front limit)";
        m_status       = ServiceStatus::Running;
        m_isDeckMoving = true;
        m_deckMotor->moveDirection(static_cast<HAL::Act::MotorDirection>(VisionDirection::DeckFront), m_deckProfile);
    }

    void VisionService::moveManual(VisionMotor motor, VisionDirection dir, bool granular)
    {
        auto it = m_cameraMotors.find(motor);
        if (it == m_cameraMotors.end() || !it->second.motor)
            return;
        vision_motor_config_t &conf = it->second;

        if (inCollisionZone(motor, dir))
        {
            if (m_pushingModeEnabled)
                applyPush(motor, conf.fineMode, granular); // Instantly apply push to the other motor to prevent stutter
            else
            {
                qWarning() << "VisionService: Movement rejected. Cameras are too close.";
                return;
            }
        }

        const auto kinematics = conf.fineMode ? conf.fineProfile : conf.fastProfile;

        if (granular)
        {
            const double relativeMovementMm = std::fabs(conf.granularMovementMm) * (dir == VisionDirection::Positive ? 1.0 : -1.0);

            conf.motor->moveRelative(relativeMovementMm, kinematics);
        }
        else
        {
            conf.currentDir = dir;
            if (conf.watchdogTicks == 0)
                conf.motor->moveDirection(static_cast<HAL::Act::MotorDirection>(dir), kinematics);
            // Reset watchdog
            conf.watchdogTicks = VISION_WATCHDOG_TIMEOUT_TICKS;
        }
    }

    void VisionService::stopManual(VisionMotor motor)
    {
        auto it = m_cameraMotors.find(motor);

        if (it == m_cameraMotors.end() || !it->second.motor)
        {
            return;
        }

        const VisionDirection lastDir = it->second.currentDir;

        // If the motor is currently being pushed by the other camera, ignore the stop command.
        // It must continue moving to avoid a hardware collision. It stops when the pushing motor stops.
        if (isMotorBeingPushed(motor, lastDir))
        {
            return;
        }

        // Stop the requested motor immediately
        it->second.watchdogTicks = 0;
        it->second.motor->emergencyStop();

        // If this motor was actively pushing another, soft-stop the pushed partner (anti-coasting safety)
        if (auto pushedPair = getAssociatedPushedMotor(motor, lastDir))
        {
            applyAntiCoastingStop(pushedPair->first, pushedPair->second);
        }
    }

    void VisionService::setKinematicMode(VisionMotor motor, bool fineMode)
    {
        auto it = m_cameraMotors.find(motor);
        if (it == m_cameraMotors.end() || !it->second.motor)
            return;

        vision_motor_config_t &conf = it->second;

        if (conf.fineMode == fineMode)
            return; // No change needed

        conf.fineMode = fineMode;

        // If the motor is currently moving, resend the order to update the hardware sent instruction immediately
        if (conf.watchdogTicks > 0)
        {
            conf.motor->moveDirection(
                static_cast<HAL::Act::MotorDirection>(conf.currentDir),
                conf.fineMode ? conf.fineProfile : conf.fastProfile);
        }
    }

    void VisionService::setFocalValue(const std::string &focalId, uint16_t val)
    {
        constexpr auto ERR_PREFIX = "[VisionService] Failed to update focal value:";
        auto focal                = m_registry->get<HAL::Act::IFocal>(focalId);

        if (focal)
        {
            if (auto it = m_focalConfs.find(focalId); it != m_focalConfs.end())
            {
                auto conf        = it->second;
                uint16_t safeVal = std::clamp(val, (uint16_t)(conf.min_value), (uint16_t)(conf.max_value));

                focal->setValueFraction(static_cast<double>(safeVal) / conf.max_value);
            }
            else
            {
                qCritical().noquote() << ERR_PREFIX << "Could not find matching configuration for identifier" << focalId;
            }
        }
        else
        {
            qCritical().noquote() << ERR_PREFIX << focal.unwrap_err();
        }
    }

    void VisionService::setFocalEnabled(const std::string &focalId, bool enabled)
    {
        constexpr auto ERR_PREFIX = "[VisionService] Failed to toggle focal enabled state:";
        auto focal                = m_registry->get<HAL::Act::IFocal>(focalId);

        if (focal)
        {
            enabled ? focal->enable() : focal->disable();
        }
        else
        {
            qCritical().noquote() << ERR_PREFIX << focal.unwrap_err();
        }
    }

    // ==========================================
    // ANTI-COLLISION & ANTI-COASTING HELPERS
    // ==========================================

    bool VisionService::inCollisionZone(VisionMotor motor, VisionDirection dir) const
    {
        // We only care about X-axis movements for collision
        if (motor != VisionMotor::UpperLeftCameraX && motor != VisionMotor::UpperRightCameraX)
            return false;

        auto itLeft  = m_cameraMotors.find(VisionMotor::UpperLeftCameraX);
        auto itRight = m_cameraMotors.find(VisionMotor::UpperRightCameraX);

        if (itLeft == m_cameraMotors.end() || !itLeft->second.motor ||
            itRight == m_cameraMotors.end() || !itRight->second.motor)
        {
            return true; // Hardware missing, fake collision risk for safety
        }

        // Fetch current positions & compute distance in mm
        const double posLeftX          = itLeft->second.motor->getEncoderPositionMm();
        const double posRightX         = itRight->second.motor->getEncoderPositionMm();
        const double currentDistanceMm = std::fabs(posRightX - posLeftX);

        // Distance is safely above the threshold, no danger
        if (currentDistanceMm > m_conf.min_camera_distance_mm)
            return false;

        // Check direction depending on which camera moves
        if (motor == VisionMotor::UpperLeftCameraX)
            return dir == VisionDirection::UpperLeftCamXRight;
        else if (motor == VisionMotor::UpperRightCameraX)
            return dir == VisionDirection::UpperRightCamXLeft;
        return false;
    }

    void VisionService::applyPush(VisionMotor pushingMotor, bool fineMode, bool granular)
    {
        VisionMotor pushedMotor;
        VisionDirection pushedDir;

        // Map the action to the pushed motor
        if (pushingMotor == VisionMotor::UpperLeftCameraX)
        {
            pushedMotor = VisionMotor::UpperRightCameraX;
            pushedDir   = VisionDirection::UpperRightCamXRight; // Moving Left Cam Right pushes Right Cam Right
        }
        else if (pushingMotor == VisionMotor::UpperRightCameraX)
        {
            pushedMotor = VisionMotor::UpperLeftCameraX;
            pushedDir   = VisionDirection::UpperLeftCamXLeft; // Moving Right Cam Left pushes Left Cam Left
        }
        else
            return;

        auto it = m_cameraMotors.find(pushedMotor);
        if (it == m_cameraMotors.end() || !it->second.motor)
            return;

        vision_motor_config_t &conf = it->second;
        const auto kinematics       = conf.fineMode ? conf.fineProfile : conf.fastProfile;

        if (granular)
        {
            const double relativeMovementMm = std::fabs(conf.granularMovementMm) * (pushedDir == VisionDirection::Positive ? 1.0 : -1.0);

            conf.motor->moveRelative(relativeMovementMm, kinematics);
        }
        else
        {
            conf.currentDir = pushedDir;
            conf.fineMode   = fineMode; // Match the kinematic profile of the pushing motor

            // Start motor if not already moving
            if (conf.watchdogTicks == 0)
            {
                conf.motor->moveDirection(static_cast<HAL::Act::MotorDirection>(pushedDir), kinematics);
            }

            // Feed the watchdog synchronously with the pushing motor
            conf.watchdogTicks = VISION_WATCHDOG_TIMEOUT_TICKS;
        }
    }

    bool VisionService::isMotorBeingPushed(VisionMotor motor, VisionDirection lastDir) const
    {
        if (!m_pushingModeEnabled)
        {
            return false;
        }

        // Left camera moving left can be pushed by Right camera moving left
        if (motor == VisionMotor::UpperLeftCameraX && lastDir == VisionDirection::UpperLeftCamXLeft)
        {
            auto rightIt = m_cameraMotors.find(VisionMotor::UpperRightCameraX);
            return (rightIt != m_cameraMotors.end() &&
                    rightIt->second.watchdogTicks > 0 &&
                    rightIt->second.currentDir == VisionDirection::UpperRightCamXLeft &&
                    inCollisionZone(VisionMotor::UpperRightCameraX, VisionDirection::UpperRightCamXLeft));
        }

        // Right camera moving right can be pushed by Left camera moving right
        if (motor == VisionMotor::UpperRightCameraX && lastDir == VisionDirection::UpperRightCamXRight)
        {
            auto leftIt = m_cameraMotors.find(VisionMotor::UpperLeftCameraX);
            return (leftIt != m_cameraMotors.end() &&
                    leftIt->second.watchdogTicks > 0 &&
                    leftIt->second.currentDir == VisionDirection::UpperLeftCamXRight &&
                    inCollisionZone(VisionMotor::UpperLeftCameraX, VisionDirection::UpperLeftCamXRight));
        }

        return false;
    }

    std::optional<std::pair<VisionMotor, VisionDirection>> VisionService::getAssociatedPushedMotor(VisionMotor motor, VisionDirection lastDir) const
    {
        if (!m_pushingModeEnabled)
        {
            return std::nullopt;
        }

        // If Left camera moving right hits collision zone, it is pushing the Right camera
        if (motor == VisionMotor::UpperLeftCameraX && lastDir == VisionDirection::UpperLeftCamXRight && inCollisionZone(motor, lastDir))
        {
            return std::make_pair(VisionMotor::UpperRightCameraX, VisionDirection::UpperRightCamXRight);
        }

        // If Right camera moving left hits collision zone, it is pushing the Left camera
        if (motor == VisionMotor::UpperRightCameraX && lastDir == VisionDirection::UpperRightCamXLeft && inCollisionZone(motor, lastDir))
        {
            return std::make_pair(VisionMotor::UpperLeftCameraX, VisionDirection::UpperLeftCamXLeft);
        }

        return std::nullopt;
    }

    void VisionService::applyAntiCoastingStop(VisionMotor pushedMotor, VisionDirection pushedDir)
    {
        auto targetIt = m_cameraMotors.find(pushedMotor);

        if (targetIt != m_cameraMotors.end() && targetIt->second.currentDir == pushedDir)
        {
            targetIt->second.watchdogTicks = 0;
            targetIt->second.motor->emergencyStop();
        }
    }

    // ==========================================
    // DECK MOVEMENT HELPER
    // ==========================================

    bool VisionService::deckVisualisationLimitReached(void) const
    {
        return HAL::MS::readBool(m_repo, DECK_FRONT_LIMIT);
    }

}
