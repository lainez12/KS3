#include <QtLogging>

#include <ApplicationBuilder.h>
#include <Config/helper.h>

// Include the Controllers and ViewModels
#include <Controllers/MotorTestController.h>
#include <Controllers/ProcedureTestController.h>
#include <HAL/MachineStatus/actuators_labels.h>
#include <HAL/Vision/identifiers.h>
#include <ViewModels/MotorTestViewModel.h>
#include <ViewModels/ProcedureTestViewModel.h>
#include <Views/CamerasTestView.h>
#include <Views/MotorTestView.h>
#include <Views/ProcedureTestView.h>

namespace Kub3::Tools::Tester
{

    void ApplicationBuilder::loadConfigurations(const std::string &hwPath, const std::string &processPath)
    {
        qInfo() << "[Tester] Loading Configs...";
        m_hwConfig      = Config::ConfigLoader::loadHardwareConfig(hwPath);
        m_processConfig = Config::ConfigLoader::loadProcessConfig(processPath);

        qInfo() << m_hwConfig;
        qInfo() << m_processConfig;
    }

    void ApplicationBuilder::buildHardwareTier()
    {
        qInfo() << "[Tester] Booting Hardware Manager...";
        m_repo      = std::make_shared<HAL::MS::MachineStatusRepo>();
        m_hwManager = std::make_unique<HAL::HardwareManager>(m_repo, m_hwConfig);
    }

    void ApplicationBuilder::buildLogicTier()
    {
        qInfo() << "[Tester] Building Logic Tier...";
        qRegisterMetaType<Kub3::Config::kinematic_profile_t>("Kub3::Config::kinematic_profile_t");

        auto registry = m_hwManager->getActuatorRegistry();

        // Create Qt Controller wrappers and Thread
        m_logicThread             = new QThread(qApp);
        m_motorTestController     = new MotorTestController(registry, m_hwManager->getRegisteredMotorIds());
        m_camerasTestController   = new CameraTestController(registry, m_hwManager->getRegisteredFocalIds(), m_hwManager->getRegisteredLightIds());
        m_procedureTestController = new ProcedureTestController(registry, m_repo, m_processConfig, m_hwConfig);

        // Move to Tier 2 execution thread
        m_motorTestController->moveToThread(m_logicThread);
        m_camerasTestController->moveToThread(m_logicThread);
        m_procedureTestController->moveToThread(m_logicThread);
    }

    void ApplicationBuilder::buildUserInterfaceTier()
    {
        qInfo() << "[Tester] Building UI...";

        // Create Main Window
        m_mainWindow = std::make_unique<TesterMainWindow>();
        // Create ViewModels
        m_motorTestViewModel     = std::make_shared<MotorTestViewModel>();
        m_procedureTestViewModel = std::make_shared<ProcedureTestViewModel>(m_repo);
        m_cameraTestViewModel    = std::make_shared<CamerasTestViewModel>();

        // Create Views and Inject ViewModels
        auto *motorTestView     = new MotorTestView(m_motorTestViewModel, m_mainWindow.get());
        auto *procedureTestView = new ProcedureTestView(m_procedureTestViewModel, m_hwConfig, m_mainWindow.get());
        auto *cameraTestView    = new CamerasTestView(m_cameraTestViewModel, m_mainWindow.get());

        // Register Views as Tabs
        m_mainWindow->addView("Single Motor", motorTestView);
        m_mainWindow->addView("Cameras", cameraTestView);
        m_mainWindow->addView("Procedures", procedureTestView);

        // Populate catalog
        // --- Motors
        {
            QList<QPair<QString, bool>> availableMotors;

            for (const std::string &motorKey : m_hwManager->getRegisteredMotorIds())
            {
                bool isStepper = false;
                auto qMotorKey = QString::fromStdString(motorKey);

                if (auto it = m_hwConfig.motors.find(qMotorKey); it != m_hwConfig.motors.end())
                    isStepper = std::holds_alternative<Config::stepper_hw_properties_t>(it->second.hwProperties);
                availableMotors.append({qMotorKey, isStepper});
            }
            m_motorTestViewModel->setAvailableMotors(availableMotors);
        }
        // --- Cameras
        {
            QList<CameraModuleConfig> cameraConfigs;

            for (const auto &[camId, camConf] : m_hwConfig.cameras)
            {
                cameraConfigs.append({
                    .cameraId = camId,
                    .focalId  = camConf.associatedFocalId,
                    .lightId  = camConf.associatedLightId,
                });
            }

            m_cameraTestViewModel->setCameraModules(cameraConfigs);
        }
    }

    void ApplicationBuilder::wireArchitecture()
    {
        qInfo() << "[Tester] Wiring strictly decoupled architecture...";

        // Wire the Global Emergency-Stop to ALL ViewModels
        QObject::connect(m_mainWindow.get(), &TesterMainWindow::s_globalEmergencyStopRequested, m_motorTestViewModel.get(), &MotorTestViewModel::uiEmergencyStop);
        QObject::connect(m_mainWindow.get(), &TesterMainWindow::s_globalEmergencyStopRequested, m_procedureTestViewModel.get(), &ProcedureTestViewModel::uiEmergencyStop);
        QObject::connect(m_mainWindow.get(), &TesterMainWindow::s_globalEmergencyStopRequested, m_cameraTestViewModel.get(), &CamerasTestViewModel::uiEmergencyStop);

        // ==========================================
        // SINGLE MOTOR TESTER WIRING
        // ==========================================

        // Machine repository (HAL) -> Controller (Logic Thread)
        QObject::connect(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_machineValueChanged,
                         m_motorTestController, &Tools::Tester::MotorTestController::ps_onMachineStatusUpdate);

        // ViewModel (Main Thread) -> Controller (Logic Thread)
        QObject::connect(m_motorTestViewModel.get(), &MotorTestViewModel::cmdSelectMotor, m_motorTestController, &MotorTestController::ps_selectMotor, Qt::QueuedConnection);
        QObject::connect(m_motorTestViewModel.get(), &MotorTestViewModel::cmdJog, m_motorTestController, &MotorTestController::ps_startJog, Qt::QueuedConnection);
        QObject::connect(m_motorTestViewModel.get(), &MotorTestViewModel::cmdStopJog, m_motorTestController, &MotorTestController::ps_stopJog, Qt::QueuedConnection);
        QObject::connect(m_motorTestViewModel.get(), &MotorTestViewModel::cmdMoveToAbsolute, m_motorTestController, &MotorTestController::ps_moveToAbsolute, Qt::QueuedConnection);
        QObject::connect(m_motorTestViewModel.get(), &MotorTestViewModel::cmdEmergencyStopAll, m_motorTestController, &MotorTestController::ps_emergencyStopAll, Qt::QueuedConnection);

        // Controller (Logic Thread) -> ViewModel (Main Thread)
        QObject::connect(m_motorTestController, &MotorTestController::s_telemetryUpdated, m_motorTestViewModel.get(), &MotorTestViewModel::onTelemetryUpdated, Qt::QueuedConnection);
        QObject::connect(m_motorTestController, &MotorTestController::s_motorSelectionChanged, m_motorTestViewModel.get(), &MotorTestViewModel::onMotorSelectionChanged, Qt::QueuedConnection);

        // ==========================================
        // PROCEDURE TESTER WIRING
        // ==========================================
        auto procVM = m_procedureTestViewModel.get();

        // ViewModel (Main Thread) -> Controller (Logic Thread)
        QObject::connect(procVM, &ProcedureTestViewModel::cmdEmergencyStop, m_procedureTestController, &ProcedureTestController::ps_emergencyStop, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitStages, m_procedureTestController, &ProcedureTestController::ps_runInitStages, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunCenterStages, m_procedureTestController, &ProcedureTestController::ps_runCenterStages, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitCameras, m_procedureTestController, &ProcedureTestController::ps_runInitCameras, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitDeck, m_procedureTestController, &ProcedureTestController::ps_runInitDeck, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitVision, m_procedureTestController, &ProcedureTestController::ps_runInitVision, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitDrawer, m_procedureTestController, &ProcedureTestController::ps_runInitDrawer, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunHomeDrawer, m_procedureTestController, &ProcedureTestController::ps_runHomeDrawer, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunHoming, m_procedureTestController, &ProcedureTestController::ps_runHoming, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunDrawerOperation, m_procedureTestController, &ProcedureTestController::ps_runDrawerOperation, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunStowage, m_procedureTestController, &ProcedureTestController::ps_runStowage, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunUnstowage, m_procedureTestController, &ProcedureTestController::ps_runUnstowage, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitZAxes, m_procedureTestController, &ProcedureTestController::ps_runInitZAxes, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunHomeZAxes, m_procedureTestController, &ProcedureTestController::ps_runHomeZAxes, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunAutolevel, m_procedureTestController, &ProcedureTestController::ps_runAutolevel, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdForceSensorTare, m_procedureTestController, &ProcedureTestController::ps_forceSensorTare, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdForceSensorsToggle, m_procedureTestController, &ProcedureTestController::ps_toggleForceSensors, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunCameraMovement, m_procedureTestController, &ProcedureTestController::ps_runCameraMovement, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunAlignmentStageMovement, m_procedureTestController, &ProcedureTestController::ps_runAlignmentStageMovement, Qt::QueuedConnection);

        // Controller (Logic Thread) -> ViewModel (Main Thread)
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureStarted, procVM, &ProcedureTestViewModel::onProcedureStarted, Qt::QueuedConnection);
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureCompleted, procVM, &ProcedureTestViewModel::onProcedureCompleted, Qt::QueuedConnection);
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureFailed, procVM, &ProcedureTestViewModel::onProcedureFailed, Qt::QueuedConnection);
        // HAL -> ViewModel
        QObject::connect(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_machineValueChanged, procVM, &ProcedureTestViewModel::onMachineValueChanged, Qt::QueuedConnection);

        // ==========================================
        // CAMERAS TESTER WIRING
        // ==========================================

        // ViewModel (Main Thread) -> Controller (Logic Thread)
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::s_exposureSliderValueChanged, m_camerasTestController, &CameraTestController::ps_requestExposureUpdate);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::s_gainSliderValueChanged, m_camerasTestController, &CameraTestController::ps_requestGainUpdate);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::s_framerateValueChanged, m_camerasTestController, &CameraTestController::ps_requestFrameRateUpdate);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::s_centeredZoomValueChanged, m_camerasTestController, &CameraTestController::ps_requestCenteredZoomUpdate);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::s_roiChanged, m_camerasTestController, &CameraTestController::ps_requestROIUpdate);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::cmdToggleFocal, m_camerasTestController, &CameraTestController::ps_toggleFocal, Qt::QueuedConnection);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::cmdUpdateFocalValue, m_camerasTestController, &CameraTestController::ps_updateFocalValue, Qt::QueuedConnection);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::cmdToggleLight, m_camerasTestController, &CameraTestController::ps_toggleLight, Qt::QueuedConnection);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::cmdUpdateLightValue, m_camerasTestController, &CameraTestController::ps_updateLightValue, Qt::QueuedConnection);
        QObject::connect(m_cameraTestViewModel.get(), &CamerasTestViewModel::cmdEmergencyStop, m_camerasTestController, &CameraTestController::ps_emergencyStop, Qt::QueuedConnection);
        // Controller -> Cameras
        QObject::connect(m_camerasTestController, &CameraTestController::s_cameraUpdate, m_hwManager.get(), &HAL::HardwareManager::ps_updateCameraParameter);

        m_cameraTestViewModel->bindConnection(
            m_hwManager.get(), &HAL::HardwareManager::s_cameraFrameReady,
            m_cameraTestViewModel.get(), &CamerasTestViewModel::ps_onCameraFrameReceived);

        // ==========================================
        // THREAD LIFECYCLE WIRING
        // ==========================================

        QObject::connect(m_logicThread, &QThread::started, m_motorTestController, &MotorTestController::start);
        QObject::connect(m_logicThread, &QThread::started, m_procedureTestController, &ProcedureTestController::start);
        QObject::connect(m_logicThread, &QThread::started, m_camerasTestController, &CameraTestController::start);

        QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_logicThread, &QThread::quit);

        QObject::connect(m_logicThread, &QThread::finished, m_motorTestController, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_procedureTestController, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_camerasTestController, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_logicThread, &QObject::deleteLater);
    }

    int ApplicationBuilder::run(QApplication &app)
    {
        qInfo() << "[Tester] Starting Hardware Manager.";
        m_hwManager->startAll();

        qInfo() << "[Tester] Starting Logic Thread.";
        m_logicThread->start();

        m_mainWindow->show();

        const int ret = app.exec();

        qInfo() << "[Tester] Shutting down Hardware Manager.";
        m_hwManager->stopAll();

        return ret;
    }

} // namespace Kub3::Tools::Tester
