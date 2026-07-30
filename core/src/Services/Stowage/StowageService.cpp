#include <QDebug>

#include <HAL/Actuators/Motors/IMotor.h>
#include <HAL/Actuators/Valves/IValve.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <Services/Stowage/StowageService.h>
#include <Services/Stowage/tasks/StowageCenterStagesTask.h>
#include <Services/Stowage/tasks/StowageEnableMaskVacuumWhenNeededTask.h>
#include <Services/Stowage/tasks/StowageMaskToArdkoCountPositionTask.h>
#include <Services/Stowage/tasks/StowageMoveMaskConvToUnconstrainedTask.h>
#include <Services/Stowage/tasks/StowageMoveZToLimitTask.h>
#include <Services/Stowage/tasks/StowageRecordMaskPositionsTask.h>
#include <Services/Stowage/tasks/StowageWaitForMaskVacuumTask.h>
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

        UNWRAP_OR_THROW(waferVacuumValve, m_registry->get<HAL::Act::IValve>(WAFER_VACUUM_VALVE), "[StowageService] Failed to load Wafer Vacuum Valve: ");
        UNWRAP_OR_THROW(maskVacuumValve, m_registry->get<HAL::Act::IValve>(MASK_VACUUM_VALVE), "[StowageService] Failed to load Mask Vacuum Valve: ");

        m_waferVacuumValve = waferVacuumValve;
        m_maskVacuumValve  = maskVacuumValve;
    }

    void StowageService::startStowage(StowageTarget target)
    {
        this->clearTasks();
        if ((target & StowageTarget::Mask) != StowageTarget::None)
        {
            if (!buildMaskStowageTaskQueue())
                return;
        }
        if ((target & StowageTarget::Wafer) != StowageTarget::None)
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
        if (allArdkoActive() && HAL::MS::readBool(m_repo, MASK_VACUUM_ACTIVE)) // Check if already loaded
            return true;                                                       // Nothing to do
        if (HAL::MS::readBool(m_repo, CM3))
        {
            postWarning("No mask holder detected. Skipping.");
            return true;
        }
        if (!HAL::MS::readBool(m_repo, CM2))
        {
            abortSequence("Stowage procedure rejected: Mask drawer not is not inserted.");
            return false;
        }

        if (anyArdkoActive())
        {
            enqueueTask<StowageMaskToArdkoCountPositionTask, 0>(m_repo, 0, m_maskBundle); // Go back to no ardko active
        }
        enqueueTask<StowageMaskToArdkoCountPositionTask, 0>(m_repo, 4, m_maskBundle); // Climb to vacuuming position (4 ardkos active)
        enqueueTask<StowageWaitForMaskVacuumTask, 0>(m_repo);
        enqueueTask<StowageMoveMaskConvToUnconstrainedTask, 0>(m_repo, m_maskBundle);
        enqueueTask<StowageEnableMaskVacuumWhenNeededTask, 1>(m_repo, m_maskVacuumValve, m_maskBundle);
        enqueueTask<StowageRecordMaskPositionsTask, 2>(m_repo, m_maskBundle);

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
        {
            return true; // Nothing to do
        }

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
        m_repo->setValueRaw(V_MASK_CONV_UNCONSTRAINED_POS_MM, static_cast<double>(0.0));
        m_repo->setValueRaw(V_MASK_CONV_RECEPTION_POS_MM, static_cast<double>(0.0));
        m_repo->setValueRaw(V_TARE_FORCE_LEFT_ADC, static_cast<uint16_t>(0));
        m_repo->setValueRaw(V_TARE_FORCE_RIGHT_ADC, static_cast<uint16_t>(0));
        m_repo->setValueRaw(V_TARE_FORCE_BACK_ADC, static_cast<uint16_t>(0));
    }

    void StowageService::initializeMotorsBundles(void)
    {
        UNWRAP_OR_THROW(leftMotor, m_registry->get<HAL::Act::IPositionMotor>(Z_LEFT_MOTOR), "[StowageService] Failed to load Z Left Motor: ");
        UNWRAP_OR_THROW(rightMotor, m_registry->get<HAL::Act::IPositionMotor>(Z_RIGHT_MOTOR), "[StowageService] Failed to load Z Right Motor: ");
        UNWRAP_OR_THROW(backMotor, m_registry->get<HAL::Act::IPositionMotor>(Z_BACK_MOTOR), "[StowageService] Failed to load Z Back Motor: ");
        UNWRAP_OR_THROW(xMotor, m_registry->get<HAL::Act::IPositionMotor>(X_STAGE_MOTOR), "[StowageService] Failed to load X Stage Motor: ");
        UNWRAP_OR_THROW(yMotor, m_registry->get<HAL::Act::IPositionMotor>(Y_STAGE_MOTOR), "[StowageService] Failed to load Y Stage Motor: ");
        UNWRAP_OR_THROW(thetaMotor, m_registry->get<HAL::Act::IPositionMotor>(THETA_STAGE_MOTOR), "[StowageService] Failed to load Theta Stage Motor: ");
        UNWRAP_OR_THROW(maskMotor, m_registry->get<HAL::Act::IPositionMotor>(MASK_DRAWER_MOTOR), "[StowageService] Failed to load Mask Conveyor Motor: ");

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
        m_maskBundle = stowage_mask_motor_bundle_t{
            .motor      = maskMotor,
            .kinematics = m_conf.getKinematicProfile(MASK_DRAWER_MOTOR, "fine"),
        };
    }

    bool StowageService::allArdkoActive(void)
    {
        return HAL::MS::readBool(m_repo, ARDKO_BACK_LEFT_LIMIT) &&
               HAL::MS::readBool(m_repo, ARDKO_BACK_RIGHT_LIMIT) &&
               HAL::MS::readBool(m_repo, ARDKO_FRONT_LEFT_LIMIT) &&
               HAL::MS::readBool(m_repo, ARDKO_FRONT_RIGHT_LIMIT);
    }

    bool StowageService::anyArdkoActive(void)
    {
        return HAL::MS::readBool(m_repo, ARDKO_BACK_LEFT_LIMIT) ||
               HAL::MS::readBool(m_repo, ARDKO_BACK_RIGHT_LIMIT) ||
               HAL::MS::readBool(m_repo, ARDKO_FRONT_LEFT_LIMIT) ||
               HAL::MS::readBool(m_repo, ARDKO_FRONT_RIGHT_LIMIT);
    }

}