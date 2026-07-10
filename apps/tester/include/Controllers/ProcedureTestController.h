#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <vector>

#include <Common/Enums.h>
#include <Config/conf.h>
#include <HAL/Actuators/ActuatorRegistry.h>
#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <Services/Alignment/IAlignmentService.h>
#include <Services/Contact/IContactService.h>
#include <Services/Drawers/IDrawerService.h>
#include <Services/Exposure/IExposureService.h>
#include <Services/Homing/IHomingService.h>
#include <Services/IService.h>
#include <Services/Stowage/IStowageService.h>
#include <Services/Vision/IVisionService.h>
#include <utils.h>

namespace Kub3::Tools::Tester
{

    class ProcedureTestController : public QObject
    {
        Q_OBJECT

    public:
        explicit ProcedureTestController(Shared<HAL::Act::ActuatorRegistry> registry,
                                         Shared<HAL::MS::IMachineStatusRepo> repo,
                                         Config::process_config_t processConfig,
                                         Config::hardware_config_t hwConfig,
                                         QObject *parent = nullptr);

        ~ProcedureTestController() override;

    public slots:
        // System lifecycle
        void start();
        void stop();

        // Commands from ViewModel
        void ps_emergencyStop(void);
        void ps_runInitStages(void);
        void ps_runCenterStages(void);
        void ps_runInitCameras(void);
        void ps_runInitDeck(void);
        void ps_runInitVision(void);
        void ps_runHoming(int targetBits);
        void ps_runDrawerOperation(DrawerTarget target, bool eject);
        void ps_runStowage(int targetInt);
        void ps_runUnstowage(int targetInt);
        void ps_runInitZAxes(void);
        void ps_runHomeZAxes(void);
        void ps_runAutolevel(void);
        void ps_runCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir);
        void ps_runAlignmentStageMovement(AlignmentStageId stageId, MovementKind kind, AlignmentStageDirection dir);

    signals:
        // Pushed to ViewModel
        void s_procedureStarted(const QString &procedureName);
        void s_procedureCompleted(const QString &procedureName);
        void s_procedureFailed(const QString &procedureName, const QString &reason);

    private slots:
        void onTick();

    private:
        void startServiceRoutine(Services::IService *service, const QString &procedureName, bool noEmit = false);

        QTimer m_tickTimer;
        Services::IService *m_activeService = nullptr;
        QString m_currentProcedureName;

        Shared<Services::IHomingService> m_homingService;
        Shared<Services::IDrawerService> m_drawerService;
        Shared<Services::IStowageService> m_stowageService;
        Shared<Services::IAlignmentService> m_alignmentService;
        Shared<Services::IVisionService> m_visionService;
        Shared<Services::IContactService> m_contactService;
        Shared<Services::IExposureService> m_exposureService;
        std::vector<Services::IService *> m_allServices;

        Shared<HAL::Act::ActuatorRegistry> m_registry;
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
        Config::process_config_t m_processConfig;
        Config::hardware_config_t m_hwConfig;

        struct CameraMovement {
            Services::VisionMotor motor;
            Services::VisionDirection dir;
        };
        std::vector<CameraMovement> m_activeCameraMoves;
        struct AlignmentStageMovement {
            AlignmentStageId motor;
            Services::AlignmentDirection dir;
        };
        std::vector<AlignmentStageMovement> m_activeAlignmentStageMoves;
    };

} // namespace Kub3::Tools::Tester
