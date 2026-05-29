#include <ApplicationBuilder.h>
#include <QDebug>

// Include the Controllers and ViewModels
#include <controllers/MotorTestController.h>
#include <controllers/ProcedureTestController.h>
#include <view_models/MotorTestViewModel.h>
#include <view_models/ProcedureTestViewModel.h>
#include <views/MotorTestView.h>
#include <views/ProcedureTestView.h>

namespace Kub3::Tools::Tester
{

    void ApplicationBuilder::loadConfigurations(const std::string &hwPath, const std::string &processPath)
    {
        qInfo() << "[Tester] Loading Configs...";
        m_hwConfig      = Config::ConfigLoader::loadHardwareConfig(hwPath);
        m_processConfig = Config::ConfigLoader::loadProcessConfig(processPath);
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

        // Get list of available motors and inject them in the service
        std::vector<std::string> knownMotorsIds;
        for (const auto &[motorKey, _] : m_hwConfig.motors)
        {
            knownMotorsIds.push_back(motorKey.toStdString());
        }

        auto registry = m_hwManager->getActuatorRegistry();

        // Create Qt Controller wrappers and Thread
        m_logicThread             = new QThread(qApp);
        m_motorTestController     = new MotorTestController(registry, knownMotorsIds);
        m_procedureTestController = new ProcedureTestController(registry, m_repo, m_processConfig, m_hwConfig);

        // Move to Tier 2 execution thread
        m_motorTestController->moveToThread(m_logicThread);
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

        // Create Views and Inject ViewModels
        auto *motorTestView     = new MotorTestView(m_motorTestViewModel, m_mainWindow.get());
        auto *procedureTestView = new ProcedureTestView(m_procedureTestViewModel, m_mainWindow.get());
        // auto* focalTestView = new FocalTestView(m_focalTestViewModel, m_mainWindow.get());

        // Register Views as Tabs
        m_mainWindow->addView("Single Motor", motorTestView);
        m_mainWindow->addView("Procedures", procedureTestView);
        // m_mainWindow->addView("Focal Config", focalTestView);

        // Populate catalog
        QList<QPair<QString, bool>> availableMotors;
        for (const auto &[motorKey, motorConfig] : m_hwConfig.motors)
        {
            bool isStepper = std::holds_alternative<Config::stepper_hw_properties_t>(motorConfig.hwProperties);
            availableMotors.append({motorKey, isStepper});
        }
        m_motorTestViewModel->setAvailableMotors(availableMotors);
    }

    void ApplicationBuilder::wireArchitecture()
    {
        qInfo() << "[Tester] Wiring strictly decoupled architecture...";

        // Wire the Global Emergency-Stop to ALL ViewModels
        QObject::connect(m_mainWindow.get(), &TesterMainWindow::s_globalEmergencyStopRequested, m_motorTestViewModel.get(), &MotorTestViewModel::uiEmergencyStop);
        QObject::connect(m_mainWindow.get(), &TesterMainWindow::s_globalEmergencyStopRequested, m_procedureTestViewModel.get(), &ProcedureTestViewModel::uiEmergencyStop);

        // ==========================================
        // SINGLE MOTOR TESTER WIRING
        // ==========================================

        // Machine repository (HAL) -> Controller (Logic Thread)
        QObject::connect(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_sensorValueChanged,
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

        // ViewModel (Main Thread) -> Controller (Logic Thread)
        QObject::connect(m_procedureTestViewModel.get(), &ProcedureTestViewModel::cmdEmergencyStop, m_procedureTestController, &ProcedureTestController::ps_emergencyStop, Qt::QueuedConnection);
        QObject::connect(m_procedureTestViewModel.get(), &ProcedureTestViewModel::cmdRunHoming, m_procedureTestController, &ProcedureTestController::ps_runHoming, Qt::QueuedConnection);
        QObject::connect(m_procedureTestViewModel.get(), &ProcedureTestViewModel::cmdRunDrawerOperation, m_procedureTestController, &ProcedureTestController::ps_runDrawerOperation, Qt::QueuedConnection);
        QObject::connect(m_procedureTestViewModel.get(), &ProcedureTestViewModel::cmdRunStowage, m_procedureTestController, &ProcedureTestController::ps_runStowage, Qt::QueuedConnection);
        QObject::connect(m_procedureTestViewModel.get(), &ProcedureTestViewModel::cmdRunUnstowage, m_procedureTestController, &ProcedureTestController::ps_runUnstowage, Qt::QueuedConnection);

        // Controller (Logic Thread) -> ViewModel (Main Thread)
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureStarted, m_procedureTestViewModel.get(), &ProcedureTestViewModel::onProcedureStarted, Qt::QueuedConnection);
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureCompleted, m_procedureTestViewModel.get(), &ProcedureTestViewModel::onProcedureCompleted, Qt::QueuedConnection);
        QObject::connect(m_procedureTestController, &ProcedureTestController::s_procedureFailed, m_procedureTestViewModel.get(), &ProcedureTestViewModel::onProcedureFailed, Qt::QueuedConnection);
        // HAL -> ViewModel
        QObject::connect(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_sensorValueChanged, m_procedureTestViewModel.get(), &ProcedureTestViewModel::onHandleSensorValueChanged, Qt::QueuedConnection);

        // ==========================================
        // THREAD LIFECYCLE WIRING
        // ==========================================

        QObject::connect(m_logicThread, &QThread::started, m_motorTestController, &MotorTestController::start);
        QObject::connect(m_logicThread, &QThread::started, m_procedureTestController, &ProcedureTestController::start);

        QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_logicThread, &QThread::quit);

        QObject::connect(m_logicThread, &QThread::finished, m_motorTestController, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_procedureTestController, &QObject::deleteLater);
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
