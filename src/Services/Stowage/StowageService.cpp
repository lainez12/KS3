#include <QDebug>

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/Actuators/Valves/IValve.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <Services/Stowage/StowageService.h>

#include <Services/Stowage/tasks/StowageCenterStagesTask.h>
#include <Services/Stowage/tasks/StowageMoveZToLimitTask.h>
#include <Services/tasks/ToggleValveTask.h>

namespace Kub3::Services
{

    StowageService::StowageService(Shared<HAL::Act::ActuatorRegistry> registry,
                                   Shared<HAL::MS::IMachineStatusRepo> repo,
                                   const Config::process_config_t &config) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo)),
        m_config(config)
    {
        // TODO: split kinematic profiles per motor (requires re-architecturing)
        m_zMotorsBundle = z_motors_bundle_t{
            .leftMotor   = m_registry->get<HAL::Act::IMotor>(Z_LEFT_MOTOR),
            .rightMotor  = m_registry->get<HAL::Act::IMotor>(Z_RIGHT_MOTOR),
            .backMotor   = m_registry->get<HAL::Act::IMotor>(Z_BACK_MOTOR),
            .fastProfile = config.getKinematicProfile(Z_LEFT_MOTOR, "normal"),
            .fineProfile = config.getKinematicProfile(Z_LEFT_MOTOR, "fine"),
        };

        m_xMotorBundle = stage_motor_bundle_t{
            .motor            = m_registry->get<HAL::Act::IMotor>(X_STAGE_MOTOR),
            .kinematic        = config.getKinematicProfile(X_STAGE_MOTOR, "normal"),
            .centerPositionMm = m_config.x_stage_center_pos_mm,
        };
        m_yMotorBundle = stage_motor_bundle_t{
            .motor            = m_registry->get<HAL::Act::IMotor>(Y_STAGE_MOTOR),
            .kinematic        = config.getKinematicProfile(Y_STAGE_MOTOR, "normal"),
            .centerPositionMm = m_config.y_stage_center_pos_mm,
        };
        m_thetaMotorBundle = stage_motor_bundle_t{
            .motor            = m_registry->get<HAL::Act::IMotor>(THETA_STAGE_MOTOR),
            .kinematic        = config.getKinematicProfile(THETA_STAGE_MOTOR, "normal"),
            .centerPositionMm = m_config.theta_stage_center_pos_mm,
        };

        m_waferVacuumValve = m_registry->get<HAL::Act::IValve>(WAFER_VACUUM_VALVE);
    }

    void StowageService::startStowage(StowageTarget target)
    {
        this->clearTasks();
        if (target && StowageTarget::MASK)
        {
            if (!buildMaskStowageTaskQueue())
                return;
        }
        if (target && StowageTarget::WAFER)
        {
            if (!buildWaferStowageTaskQueue())
                return;
        }
        this->startSequence();
    }

    void StowageService::stop(void)
    {
        // Stop Z motors
        if (m_zMotorsBundle.leftMotor)
            m_zMotorsBundle.leftMotor->emergencyStop();
        if (m_zMotorsBundle.rightMotor)
            m_zMotorsBundle.rightMotor->emergencyStop();
        if (m_zMotorsBundle.backMotor)
            m_zMotorsBundle.backMotor->emergencyStop();
        // Stop alignment motors
        if (m_xMotorBundle.motor)
            m_xMotorBundle.motor->emergencyStop();
        if (m_yMotorBundle.motor)
            m_yMotorBundle.motor->emergencyStop();
        if (m_thetaMotorBundle.motor)
            m_thetaMotorBundle.motor->emergencyStop();

        BaseTaskService::stop();
    }

    bool StowageService::buildMaskStowageTaskQueue(void)
    {
        // TODO: code
        return true;
    }

    bool StowageService::buildWaferStowageTaskQueue(void)
    {
        // Mandatory Pre-Condition: Wafer drawer MUST be inserted
        if (!HAL::MS::readBool(m_repo, CW2))
        {
            abortSequence("Stowage procedure rejected: Wafer drawer is not inserted.");
            return false;
        }

        if (HAL::MS::readBool(m_repo, Z2)) // Check if already loaded
            return true;                   // Nothing to do

        if (!HAL::MS::readBool(m_repo, WAFER_ON)) // Lower than Z2 and WAFER_ON
        {
            if (HAL::MS::readBool(m_repo, Z1))
            {
                enqueueTask<StowageMoveZToLimitTask>(m_repo, m_zMotorsBundle, ZLimit::_Z1, false); // Go below Z1
            }
            enqueueTask<StowageCenterStagesTask>(m_repo, m_xMotorBundle, m_yMotorBundle, m_thetaMotorBundle); // Center alignment stages
            enqueueTask<StowageMoveZToLimitTask>(m_repo, m_zMotorsBundle, ZLimit::_WAFER_ON, true);           // Move Z UP until WAFER_ON is True
            enqueueTask<ToggleValveTask>(m_repo, m_waferVacuumValve, true, WAFER_VACUUM_ACTIVE, true);        // Turn on Vacuum
        }
        enqueueTask<StowageMoveZToLimitTask>(m_repo, m_zMotorsBundle, ZLimit::_Z2, true); // Move Z UP until Z2 is True (Entering alignment zone)

        return true;
    }

    bool StowageService::isAbsoluteBottomLimitReached() const
    {
        return HAL::MS::readBool(m_repo, Z_LEFT_LOW_LIMIT) ||
               HAL::MS::readBool(m_repo, Z_RIGHT_LOW_LIMIT) ||
               HAL::MS::readBool(m_repo, Z_BACK_LOW_LIMIT);
    }

    bool StowageService::isAbsoluteTopLimitReached() const
    {
        return HAL::MS::readBool(m_repo, Z_LEFT_HIGH_LIMIT) ||
               HAL::MS::readBool(m_repo, Z_RIGHT_HIGH_LIMIT) ||
               HAL::MS::readBool(m_repo, Z_BACK_HIGH_LIMIT);
    }
}