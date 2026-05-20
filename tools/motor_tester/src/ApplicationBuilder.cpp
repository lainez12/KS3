#include <ApplicationBuilder.h>
#include <QDebug>

namespace Kub3::Tools::MotorTester
{

    void ApplicationBuilder::loadConfigurations(const std::string &hwPath, const std::string &processPath)
    {
        qInfo() << "[MotorTester] Loading Configs...";
        m_hwConfig      = Config::ConfigLoader::loadHardwareConfig(hwPath);
        m_processConfig = Config::ConfigLoader::loadProcessConfig(processPath);
    }

    void ApplicationBuilder::buildHardwareTier()
    {
        qInfo() << "[MotorTester] Booting Hardware Manager...";
        m_repo      = std::make_shared<HAL::MS::MachineStatusRepo>();
        m_hwManager = std::make_unique<HAL::HardwareManager>(m_repo, m_hwConfig);
    }

    void ApplicationBuilder::buildLogicTier()
    {
        qInfo() << "[MotorTester] Building Logic Tier...";

        qRegisterMetaType<Kub3::Config::kinematic_profile_t>("Kub3::Config::kinematic_profile_t");

        // Get list of available motors and inject them in the service
        std::vector<std::string> knownMotorsIds;
        for (const auto &[motorKey, _] : m_hwConfig.motors)
        {
            knownMotorsIds.push_back(motorKey.toStdString());
        }

        // 1. Create pure C++ Service
        auto service = std::make_unique<MotorTestService>(m_hwManager->getActuatorRegistry(), knownMotorsIds);

        // 2. Create Qt Controller wrapper and Thread
        m_logicThread = new QThread(qApp);
        m_controller  = new MotorTestController(std::move(service));

        m_controller->moveToThread(m_logicThread);
    }

    void ApplicationBuilder::buildUserInterfaceTier()
    {
        qInfo() << "[MotorTester] Building UI...";
        m_viewModel  = std::make_shared<MotorTestViewModel>();
        m_mainWindow = std::make_unique<MotorTestWindow>(m_viewModel);

        // Get list of available motors and inject them in the view model
        QList<QPair<QString, bool>> availableMotors;
        for (const auto &[motorKey, motorConfig] : m_hwConfig.motors)
        {
            bool isStepper = std::holds_alternative<Config::stepper_hw_properties_t>(motorConfig.hwProperties);
            availableMotors.append({motorKey, isStepper});
        }
        m_viewModel->setAvailableMotors(availableMotors);
    }

    void ApplicationBuilder::wireArchitecture()
    {
        qInfo() << "[MotorTester] Wiring strictly decoupled architecture...";

        // Machine repository (HAL) -> Controller (Logic Thread)
        QObject::connect(m_repo.get(), &HAL::MS::IMachineStatusRepo::s_sensorValueChanged,
                         m_controller, &Tools::MotorTester::MotorTestController::ps_onMachineStatusUpdate);

        // ViewModel (Main Thread) -> Controller (Logic Thread)
        QObject::connect(m_viewModel.get(), &MotorTestViewModel::cmdSelectMotor, m_controller, &MotorTestController::ps_selectMotor, Qt::QueuedConnection);
        QObject::connect(m_viewModel.get(), &MotorTestViewModel::cmdJog, m_controller, &MotorTestController::ps_startJog, Qt::QueuedConnection);
        QObject::connect(m_viewModel.get(), &MotorTestViewModel::cmdStopJog, m_controller, &MotorTestController::ps_stopJog, Qt::QueuedConnection);
        QObject::connect(m_viewModel.get(), &MotorTestViewModel::cmdMoveToAbsolute, m_controller, &MotorTestController::ps_moveToAbsolute, Qt::QueuedConnection);
        QObject::connect(m_viewModel.get(), &MotorTestViewModel::cmdEmergencyStopAll, m_controller, &MotorTestController::ps_emergencyStopAll, Qt::QueuedConnection);

        // Controller (Logic Thread) -> ViewModel (Main Thread)
        QObject::connect(m_controller, &MotorTestController::s_telemetryUpdated, m_viewModel.get(), &MotorTestViewModel::onTelemetryUpdated, Qt::QueuedConnection);
        QObject::connect(m_controller, &MotorTestController::s_motorSelectionChanged, m_viewModel.get(), &MotorTestViewModel::onMotorSelectionChanged, Qt::QueuedConnection);
        // QObject::connect(m_controller, &MotorTestController::s_availableMotorsDiscovered, m_viewModel.get(), &MotorTestViewModel::onAvailableMotorsDiscovered, Qt::QueuedConnection);

        // Thread Lifecycle
        QObject::connect(m_logicThread, &QThread::started, m_controller, &MotorTestController::start);
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, m_logicThread, &QThread::quit);
        QObject::connect(m_logicThread, &QThread::finished, m_controller, &QObject::deleteLater);
        QObject::connect(m_logicThread, &QThread::finished, m_logicThread, &QObject::deleteLater);
    }

    int ApplicationBuilder::run(QApplication &app)
    {
        qInfo() << "[MotorTester] Starting Hardware Manager.";
        m_hwManager->startAll();

        qInfo() << "[MotorTester] Starting Logic Thread.";
        m_logicThread->start();

        m_mainWindow->show();

        const int ret = app.exec();

        qInfo() << "[MotorTester] Shutting down Hardware Manager.";
        m_hwManager->stopAll();

        return ret;
    }

}