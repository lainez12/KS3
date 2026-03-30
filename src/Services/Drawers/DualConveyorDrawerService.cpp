#ifdef KUB_MODEL_8

// HAL
#include "HAL/Actuators/Motors/IMotor.h"
#include "HAL/MachineStatus/actuators_labels.h"
#include "HAL/MachineStatus/sensors_labels.h"
#include "HAL/MachineStatus/utils.h"
// Services
#include "Services/Drawers/DualConveyorDrawerService.h"
#include "Services/Drawers/tasks/MaskEjectionTask.h"
#include "Services/Drawers/tasks/MaskInsertionTask.h"
#include "Services/Drawers/tasks/WaferEjectionTask.h"
#include "Services/Drawers/tasks/WaferInsertionTask.h"

namespace Kub3::Services
{

    DualConveyorDrawerService::DualConveyorDrawerService(Shared<HAL::Act::ActuatorRegistry> registry,
                                                         Shared<HAL::MS::IMachineStatusRepo> repo,
                                                         const Config::process_config_t &processConfig) :
        m_registry(std::move(registry)),
        m_repo(std::move(repo)),
        m_processConfig(processConfig)
    {
        loadMotorsKinematicProfiles();
        loadThresholds();
    }

    void DualConveyorDrawerService::insert(DrawerTarget target)
    {
        this->clearTasks();

        if (target == DrawerTarget::Wafer && !isMaskInserted())
            target = DrawerTarget::Both;

        // TODO: handle throws on get
        auto maskMotor  = m_registry->get<HAL::Act::IMotor>(MASK_DRAWER_MOTOR);
        auto waferMotor = m_registry->get<HAL::Act::IMotor>(WAFER_DRAWER_MOTOR);

        if (target == DrawerTarget::Mask || target == DrawerTarget::Both)
        {
            this->enqueueTask(std::make_unique<MaskInsertionTask>(
                maskMotor,
                m_repo,
                m_maskFastProfile,
                m_maskFineProfile,
                m_maskContactProfile));
        }

        if (target == DrawerTarget::Wafer || target == DrawerTarget::Both)
        {
            this->enqueueTask(std::make_unique<WaferInsertionTask>(
                waferMotor,
                m_repo,
                m_waferFastProfile,
                m_waferFineProfile));
        }

        this->startSequence();
    }

    void DualConveyorDrawerService::eject(DrawerTarget target)
    {
        this->clearTasks();

        if (target == DrawerTarget::Mask && !isWaferEjected())
            target = DrawerTarget::Both;

        // TODO: handle throws on get
        auto maskMotor  = m_registry->get<HAL::Act::IMotor>(MASK_DRAWER_MOTOR);
        auto waferMotor = m_registry->get<HAL::Act::IMotor>(WAFER_DRAWER_MOTOR);

        if (target == DrawerTarget::Wafer || target == DrawerTarget::Both)
        {
            this->enqueueTask(std::make_unique<WaferEjectionTask>(
                waferMotor,
                m_repo,
                m_waferFastProfile,
                m_waferFineProfile,
                m_waferEjectionFinePosThreshold));
        }

        if (target == DrawerTarget::Mask || target == DrawerTarget::Both)
        {
            this->enqueueTask(std::make_unique<MaskEjectionTask>(
                maskMotor,
                m_repo,
                m_maskFastProfile,
                m_maskFineProfile,
                m_maskEjectionFinePosThreshold));
        }

        this->startSequence();
    }

    void DualConveyorDrawerService::stop(void)
    {
        // TODO: handle throws on get
        m_registry->get<HAL::Act::IMotor>(MASK_DRAWER_MOTOR)->emergencyStop();
        m_registry->get<HAL::Act::IMotor>(WAFER_DRAWER_MOTOR)->emergencyStop();

        BaseTaskService::stop();
    }

    // Private methods

    bool DualConveyorDrawerService::isWaferEjected(void)
    {
        return HAL::MS::readBool(m_repo, CW0);
    }

    bool DualConveyorDrawerService::isMaskInserted(void)
    {
        return HAL::MS::readBool(m_repo, CM2) || HAL::MS::readBool(m_repo, CM3);
    }

    void DualConveyorDrawerService::loadMotorsKinematicProfiles(void)
    {
        m_waferFastProfile   = m_processConfig.getKinematicProfile(WAFER_DRAWER_MOTOR, "fast");
        m_waferFineProfile   = m_processConfig.getKinematicProfile(WAFER_DRAWER_MOTOR, "fine");
        m_maskFastProfile    = m_processConfig.getKinematicProfile(MASK_DRAWER_MOTOR, "fast");
        m_maskFineProfile    = m_processConfig.getKinematicProfile(MASK_DRAWER_MOTOR, "fine");
        m_maskContactProfile = m_processConfig.getKinematicProfile(MASK_DRAWER_MOTOR, "contact");
    }

    void DualConveyorDrawerService::loadThresholds(void)
    {
        // TODO: code
        // m_waferEjectionFinePosThreshold = m_processConfig.getEncoderThreshold("ejection_slowdown_threshold");
        // m_maskEjectionFinePosThreshold  = m_processConfig.getEncoderThreshold("ejection_slowdown_threshold");
    }

}

#endif // KUB_MODEL_8
