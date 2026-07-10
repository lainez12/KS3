#include <unordered_map>

#include <HAL/MachineStatus/utils.h>
#include <Services/Alignment/AlignmentService.h>

namespace
{

    using Kub3::AlignmentStageId;
    using Kub3::HAL::Act::MotorDirection;
    using Kub3::Services::AlignmentDirection;

    template <typename E>
    constexpr std::size_t to_index(E e) noexcept
    {
        return static_cast<std::size_t>(std::underlying_type_t<E>(e));
    }

    // TODO: change the mechanism to get the limit ID as this is extremely error prone is the used enums are update

    static constexpr std::array<std::array<const char *, 2>, 3> stopIdMap{
        {/* --------- [0] = POSITIVE LIMIT, [1] = NEGATIVE LIMIT */
         /* X --- */ {X_STAGE_LEFT_LIMIT, X_STAGE_RIGHT_LIMIT},
         /* Y --- */ {Y_STAGE_BACK_LIMIT, Y_STAGE_FRONT_LIMIT},
         /* THETA */ {THETA_STAGE_ANTI_CLOCKWISE_LIMIT, THETA_STAGE_CLOCKWISE_LIMIT}}};

    constexpr const char *stopId(AlignmentStageId s, AlignmentDirection d) noexcept
    {
        return stopIdMap[to_index(s)][to_index(d)];
    }

}

namespace Kub3::Services
{

    AlignmentService::AlignmentService(Shared<HAL::Act::ActuatorRegistry> registry,
                                       Shared<HAL::MS::IMachineStatusRepo> repo,
                                       const Config::process_config_t &processConfig) :
        m_repo(std::move(repo))
    {
        this->loadConfigurations(registry, processConfig);
    }

    void AlignmentService::tick(void)
    {
        for (auto &[axis, config] : m_motorsConfigurations)
        {
            if (config.watchdogTicks > 0)
            {
                config.watchdogTicks--;
                if (config.watchdogTicks == 0)
                {
                    // DEAD-MAN'S SWITCH TRIGGERED
                    qWarning() << std::format("AlignmentService: Dead-Man's switch triggered for axis {}. Halting motor.", static_cast<int>(axis)).c_str();
                    if (config.motor)
                        config.motor->emergencyStop();
                }
            }
        }
    }

    void AlignmentService::stop(void)
    {
        for (auto &[axis, config] : m_motorsConfigurations)
        {
            // Disable the dead-man's switch to prevent unexpected logic trigerring
            config.watchdogTicks = 0;
            if (config.motor)
                config.motor->emergencyStop();
        }
    }

    void AlignmentService::moveStage(AlignmentStageId axis, AlignmentDirection dir, bool granular)
    {
        if (m_isLocked)
            return; // Prevent moving when in locked state

        auto it = m_motorsConfigurations.find(axis);

        if (it == m_motorsConfigurations.end() || !it->second.motor)
            return;

        const bool limitReached               = HAL::MS::readBool(m_repo, stopId(axis, dir));
        motor_alignment_config_t &motorConfig = it->second;

        if (limitReached)
        {
            motorConfig.watchdogTicks = 0;
            if (motorConfig.motor->isMoving())
                it->second.motor->emergencyStop();
            return;
        }

        const auto &kinematics = motorConfig.fineMode ? motorConfig.fineProfile : motorConfig.fastProfile;

        if (granular)
        {
            const double relativeMovementMm = std::fabs(motorConfig.granularMoveMm) * (dir == AlignmentDirection::POSITIVE ? 1.0 : -1.0);

            motorConfig.motor->moveRelative(relativeMovementMm, kinematics);
        }
        else
        {
            if (motorConfig.watchdogTicks == 0) // Motor is stopped
            {
                motorConfig.motor->moveDirection(static_cast<HAL::Act::MotorDirection>(dir), kinematics);
            }
            // We only enable the watchdog when movement is not granular
            motorConfig.watchdogTicks = ALIGNMENT_WATCHDOG_TIMEOUT_TICKS; // Reset the watchdog
        }
    }

    void AlignmentService::stopStage(AlignmentStageId axis)
    {
        if (auto it = m_motorsConfigurations.find(axis); it != m_motorsConfigurations.end())
        {
            it->second.watchdogTicks = 0;
            it->second.motor->emergencyStop();
        }
    }

    void AlignmentService::setKinematicProfile(AlignmentStageId axis, bool fineMode)
    {
        if (auto it = m_motorsConfigurations.find(axis); it != m_motorsConfigurations.end())
        {
            it->second.fineMode = fineMode;
        }
    }

    void AlignmentService::setHardwareLock(bool locked)
    {
        const bool valueChanged = m_isLocked != locked;

        if (valueChanged)
        {
            m_isLocked = locked;
            if (m_isLocked)
                stop();
        }
    }

    void AlignmentService::loadConfigurations(Shared<HAL::Act::ActuatorRegistry> registry, const Config::process_config_t &processConfig)
    {
        UNWRAP_OR_THROW(xMotor, registry->get<HAL::Act::IPositionMotor>(X_STAGE_MOTOR), "[AlignmentService] Failed to load X Stage Motor: ");
        UNWRAP_OR_THROW(yMotor, registry->get<HAL::Act::IPositionMotor>(Y_STAGE_MOTOR), "[AlignmentService] Failed to load Y Stage Motor: ");
        UNWRAP_OR_THROW(thetaMotor, registry->get<HAL::Act::IPositionMotor>(THETA_STAGE_MOTOR), "[AlignmentService] Failed to load Theta Stage Motor: ");

        const motor_alignment_config_t xStageConfig{
            .motor          = xMotor,
            .fastProfile    = processConfig.getKinematicProfile(X_STAGE_MOTOR, "normal"),
            .fineProfile    = processConfig.getKinematicProfile(X_STAGE_MOTOR, "fine"),
            .granularMoveMm = processConfig.pad.x_stage_distance_mm,
        };
        const motor_alignment_config_t yStageConfig{
            .motor          = yMotor,
            .fastProfile    = processConfig.getKinematicProfile(Y_STAGE_MOTOR, "normal"),
            .fineProfile    = processConfig.getKinematicProfile(Y_STAGE_MOTOR, "fine"),
            .granularMoveMm = processConfig.pad.y_stage_distance_mm,
        };
        const motor_alignment_config_t thetaStageConfig{
            .motor          = thetaMotor,
            .fastProfile    = processConfig.getKinematicProfile(THETA_STAGE_MOTOR, "normal"),
            .fineProfile    = processConfig.getKinematicProfile(THETA_STAGE_MOTOR, "fine"),
            .granularMoveMm = processConfig.pad.theta_stage_distance_mm,
        };

        m_motorsConfigurations.insert({AlignmentStageId::X, std::move(xStageConfig)});
        m_motorsConfigurations.insert({AlignmentStageId::Y, std::move(yStageConfig)});
        m_motorsConfigurations.insert({AlignmentStageId::THETA, std::move(thetaStageConfig)});
    }
}