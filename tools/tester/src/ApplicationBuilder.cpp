#include <QtLogging>

#include <ApplicationBuilder.h>
#include <Config/helper.h>

// Include the Controllers and ViewModels
#include <controllers/MotorTestController.h>
#include <controllers/ProcedureTestController.h>
#include <view_models/MotorTestViewModel.h>
#include <view_models/ProcedureTestViewModel.h>
#include <views/FocalTestView.h>
#include <views/MotorTestView.h>
#include <views/ProcedureTestView.h>

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
        m_focalTestController     = new FocalTestController(registry, m_hwManager->getRegisteredFocalIds());
        m_procedureTestController = new ProcedureTestController(registry, m_repo, m_processConfig, m_hwConfig);

        // Move to Tier 2 execution thread
        m_motorTestController->moveToThread(m_logicThread);
        m_focalTestController->moveToThread(m_logicThread);
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
        m_focalTestViewModel     = std::make_shared<FocalTestViewModel>();

        // Create Views and Inject ViewModels
        auto *motorTestView     = new MotorTestView(m_motorTestViewModel, m_mainWindow.get());
        auto *procedureTestView = new ProcedureTestView(m_procedureTestViewModel, m_mainWindow.get());
        auto *focalTestView     = new FocalTestView(m_focalTestViewModel, m_mainWindow.get());

        // Register Views as Tabs
        m_mainWindow->addView("Single Motor", motorTestView);
        m_mainWindow->addView("Focals", focalTestView);
        m_mainWindow->addView("Procedures", procedureTestView);

        // Populate catalog
        QList<QPair<QString, bool>> availableMotors;
        QList<QString> availableFocals;

        for (const std::string &motorKey : m_hwManager->getRegisteredMotorIds())
        {
            bool isStepper = false;
            auto qMotorKey = QString::fromStdString(motorKey);

            if (auto it = m_hwConfig.motors.find(qMotorKey); it != m_hwConfig.motors.end())
                isStepper = std::holds_alternative<Config::stepper_hw_properties_t>(it->second.hwProperties);
            availableMotors.append({qMotorKey, isStepper});
        }
        m_motorTestViewModel->setAvailableMotors(availableMotors);
        for (const auto &focalKey : m_hwManager->getRegisteredFocalIds())
            availableFocals.push_back(QString::fromStdString(focalKey));
        m_focalTestViewModel->setAvailableFocals(availableFocals);
    }

    void ApplicationBuilder::wireArchitecture()
    {
        qInfo() << "[Tester] Wiring strictly decoupled architecture...";

        // Wire the Global Emergency-Stop to ALL ViewModels
        QObject::connect(m_mainWindow.get(), &TesterMainWindow::s_globalEmergencyStopRequested, m_motorTestViewModel.get(), &MotorTestViewModel::uiEmergencyStop);
        QObject::connect(m_mainWindow.get(), &TesterMainWindow::s_globalEmergencyStopRequested, m_procedureTestViewModel.get(), &ProcedureTestViewModel::uiEmergencyStop);
        QObject::connect(m_mainWindow.get(), &TesterMainWindow::s_globalEmergencyStopRequested, m_focalTestViewModel.get(), &FocalTestViewModel::uiEmergencyStop);

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
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitCameras, m_procedureTestController, &ProcedureTestController::ps_runInitCameras, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitDeck, m_procedureTestController, &ProcedureTestController::ps_runInitDeck, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunInitVision, m_procedureTestController, &ProcedureTestController::ps_runInitVision, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunHoming, m_procedureTestController, &ProcedureTestController::ps_runHoming, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunDrawerOperation, m_procedureTestController, &ProcedureTestController::ps_runDrawerOperation, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunStowage, m_procedureTestController, &ProcedureTestController::ps_runStowage, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunUnstowage, m_procedureTestController, &ProcedureTestController::ps_runUnstowage, Qt::QueuedConnection);
        QObject::connect(procVM, &ProcedureTestViewModel::cmdRunAutolevel, m_procedureTestController, &ProcedureTestController::ps_runAutolevel, Qt::QueuedConnection);

        // Controller (Logic Thread) -> ViewModel (Main Thread)
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureStarted, procVM, &ProcedureTestViewModel::onProcedureStarted, Qt::QueuedConnection);
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureCompleted, procVM, &ProcedureTestViewModel::onProcedureCompleted, Qt::QueuedConnection);
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureFailed, procVM, &ProcedureTestViewModel::onProcedureFailed, Qt::QueuedConnection);
        // HAL -> ViewModel
        QObject::connect(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_machineValueChanged, procVM, &ProcedureTestViewModel::onHandleSensorValueChanged, Qt::QueuedConnection);

        // ==========================================
        // FOCAL TESTER WIRING
        // ==========================================

        // ViewModel (Main Thread) -> Controller (Logic Thread)
        QObject::connect(m_focalTestViewModel.get(), &FocalTestViewModel::cmdToggleFocal, m_focalTestController, &FocalTestController::ps_toggleFocal, Qt::QueuedConnection);
        QObject::connect(m_focalTestViewModel.get(), &FocalTestViewModel::cmdUpdateFocalValue, m_focalTestController, &FocalTestController::ps_updateFocalValue, Qt::QueuedConnection);
        QObject::connect(m_focalTestViewModel.get(), &FocalTestViewModel::cmdEmergencyStop, m_focalTestController, &FocalTestController::ps_emergencyStop, Qt::QueuedConnection);

        // ==========================================
        // THREAD LIFECYCLE WIRING
        // ==========================================

        QObject::connect(m_logicThread, &QThread::started, m_motorTestController, &MotorTestController::start);
        QObject::connect(m_logicThread, &QThread::started, m_procedureTestController, &ProcedureTestController::start);
        QObject::connect(m_logicThread, &QThread::started, m_focalTestController, &FocalTestController::start);

        QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_logicThread, &QThread::quit);

        QObject::connect(m_logicThread, &QThread::finished, m_motorTestController, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_procedureTestController, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_focalTestController, &QObject::deleteLater);
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
