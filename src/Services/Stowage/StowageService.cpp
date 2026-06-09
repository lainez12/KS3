#include <QDebug>

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/Actuators/Valves/IValve.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <Services/Stowage/StowageService.h>

#include <Services/Stowage/tasks/StowageCenterStagesTask.h>
#include <Services/Stowage/tasks/StowageMoveZToLimitTask.h>
#include <Services/tasks/ToggleValveTask.h>

#define STOP_MOTOR_PTR(motor, motorId)                                                   \
    do                                                                                   \
    {                                                                                    \
        if (motor)                                                                       \
            motor->emergencyStop();                                                      \
        else                                                                             \
            qCritical().noquote() << "[StowageService] Failed to stop motor" << motorId; \
    } while (0);

namespace Kub3::Services
{

    StowageService::StowageService(Shared<HAL::Act::ActuatorRegistry> registry,
                                   Shared<HAL::MS::IMachineStatusRepo> repo,
                                   const Config::process_config_t &config) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo)),
        m_conf(config)
    {
        this->initializeMachineValues();
        this->initializeMotorsBundles();

        UNWRAP_OR_THROW(vacValve, m_registry->get<HAL::Act::IValve>(WAFER_VACUUM_VALVE), "[StowageService] Failed to load Wafer Vacuum Valve: ");
        m_waferVacuumValve = vacValve;
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

    void StowageService::onStop(void)
    {
        this->stopAllMotors();
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

    void StowageService::stopAllMotors(void)
    {
        // Stop Z motors
        STOP_MOTOR_PTR(m_zMotorsBundle.leftMotor, Z_LEFT_MOTOR);
        STOP_MOTOR_PTR(m_zMotorsBundle.rightMotor, Z_RIGHT_MOTOR);
        STOP_MOTOR_PTR(m_zMotorsBundle.backMotor, Z_BACK_MOTOR);
        // Stop alignment motors
        STOP_MOTOR_PTR(m_xMotorBundle.motor, X_STAGE_MOTOR);
        STOP_MOTOR_PTR(m_yMotorBundle.motor, Y_STAGE_MOTOR);
        STOP_MOTOR_PTR(m_thetaMotorBundle.motor, THETA_STAGE_MOTOR);
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

    void StowageService::initializeMachineValues(void)
    {
        m_repo->setValueRaw(V_ARDKO_CONTACT_MASK_POSITION, static_cast<int32_t>(0));
        m_repo->setValueRaw(V_TARE_FORCE_LEFT, static_cast<uint16_t>(0));
        m_repo->setValueRaw(V_TARE_FORCE_RIGHT, static_cast<uint16_t>(0));
        m_repo->setValueRaw(V_TARE_FORCE_BACK, static_cast<uint16_t>(0));
    }

    void StowageService::initializeMotorsBundles(void)
    {
        UNWRAP_OR_THROW(leftMotor, m_registry->get<HAL::Act::IMotor>(Z_LEFT_MOTOR), "[StowageService] Failed to load Z Left Motor: ");
        UNWRAP_OR_THROW(rightMotor, m_registry->get<HAL::Act::IMotor>(Z_RIGHT_MOTOR), "[StowageService] Failed to load Z Right Motor: ");
        UNWRAP_OR_THROW(backMotor, m_registry->get<HAL::Act::IMotor>(Z_BACK_MOTOR), "[StowageService] Failed to load Z Back Motor: ");
        UNWRAP_OR_THROW(xMotor, m_registry->get<HAL::Act::IMotor>(X_STAGE_MOTOR), "[StowageService] Failed to load X Stage Motor: ");
        UNWRAP_OR_THROW(yMotor, m_registry->get<HAL::Act::IMotor>(Y_STAGE_MOTOR), "[StowageService] Failed to load Y Stage Motor: ");
        UNWRAP_OR_THROW(thetaMotor, m_registry->get<HAL::Act::IMotor>(THETA_STAGE_MOTOR), "[StowageService] Failed to load Theta Stage Motor: ");

        // TODO: split kinematic profiles per motor (requires re-architecturing)
        m_zMotorsBundle = z_motors_bundle_t{
            .leftMotor   = leftMotor,
            .rightMotor  = rightMotor,
            .backMotor   = backMotor,
            .fastProfile = m_conf.getKinematicProfile(Z_LEFT_MOTOR, "normal"),
            .fineProfile = m_conf.getKinematicProfile(Z_LEFT_MOTOR, "fine"),
        };

        m_xMotorBundle = stage_motor_bundle_t{
            .motor            = xMotor,
            .kinematic        = m_conf.getKinematicProfile(X_STAGE_MOTOR, "normal"),
            .centerPositionMm = m_conf.alignment.x_stage_center_pos_mm,
        };
        m_yMotorBundle = stage_motor_bundle_t{
            .motor            = yMotor,
            .kinematic        = m_conf.getKinematicProfile(Y_STAGE_MOTOR, "normal"),
            .centerPositionMm = m_conf.alignment.y_stage_center_pos_mm,
        };
        m_thetaMotorBundle = stage_motor_bundle_t{
            .motor            = thetaMotor,
            .kinematic        = m_conf.getKinematicProfile(THETA_STAGE_MOTOR, "normal"),
            .centerPositionMm = m_conf.alignment.theta_stage_center_pos_mm,
        };
    }

}