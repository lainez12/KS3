#include <QThread>

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <ViewModels/ProcedureTestViewModel.h>

namespace Kub3::Tools::Tester
{

    ProcedureTestViewModel::ProcedureTestViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(std::move(repo))
    {
    }

    // ==========================================
    // UI INTENTS -> CONTROLLER COMMANDS
    // ==========================================

    void ProcedureTestViewModel::uiEmergencyStop()
    {
        // Unconditional bypass (Executes even if m_isRunning is true)
        emit cmdEmergencyStop();
    }

    void ProcedureTestViewModel::uiRequestHoming(int targetBits)
    {
        if (!m_isRunning)
            emit cmdRunHoming(targetBits);
    }

    void ProcedureTestViewModel::uiRequestDrawerOperation(DrawerTarget target, bool eject)
    {
        if (!m_isRunning)
            emit cmdRunDrawerOperation(target, eject);
    }

    void ProcedureTestViewModel::uiRequestStowage(int targetInt)
    {
        if (!m_isRunning)
            emit cmdRunStowage(targetInt);
    }

    void ProcedureTestViewModel::uiRequestUnstowage(int targetInt)
    {
        if (!m_isRunning)
            emit cmdRunUnstowage(targetInt);
    }

    void ProcedureTestViewModel::uiRequestInitialization(void)
    {
        if (!m_isRunning)
            emit cmdRunInitialization();
    }

    void ProcedureTestViewModel::uiRequestInitCameras(void)
    {
        if (!m_isRunning)
            emit cmdRunInitCameras();
    }

    void ProcedureTestViewModel::uiRequestInitDeck(void)
    {
        if (!m_isRunning)
            emit cmdRunInitDeck();
    }

    void ProcedureTestViewModel::uiRequestInitVision(void)
    {
        if (!m_isRunning)
            emit cmdRunInitVision();
    }

    void ProcedureTestViewModel::uiRequestAutolevel(void)
    {
        if (!m_isRunning)
            emit cmdRunAutolevel();
    }

    // ==========================================
    // CONTROLLER UPDATES -> UI STATE
    // ==========================================

    void ProcedureTestViewModel::onProcedureStarted(const QString &procedureName)
    {
        m_isRunning        = true;
        m_currentProcedure = procedureName;
        emit s_isRunningChanged();

        setStatus(QString("Running: %1...").arg(procedureName), false);
    }

    void ProcedureTestViewModel::onProcedureCompleted(const QString &procedureName)
    {
        m_isRunning = false;
        emit s_isRunningChanged();

        setStatus(QString("%1 completed successfully.").arg(procedureName), false);
    }

    void ProcedureTestViewModel::onProcedureFailed(const QString &procedureName, const QString &reason)
    {
        m_isRunning = false;
        emit s_isRunningChanged();

        setStatus(QString("%1 failed: %2").arg(procedureName, reason), true);
    }

    void ProcedureTestViewModel::setStatus(const QString &msg, bool isError)
    {
        m_lastStatusMessage = msg;
        m_hasError          = isError;
        emit s_statusMessageChanged();
    }

    void ProcedureTestViewModel::onHandleSensorValueChanged(const std::string &key)
    {
        if (!m_repo)
            return;

        Optional<HAL::MS::MachineValue> valueOpt = m_repo->getValueRaw(key);

        if (!valueOpt.has_value())
            return;

        auto qKey   = QString::fromStdString(key);
        auto museum = overloadedCallable(
            [&](bool v) { emit s_booleanSensorUpdate(qKey, v); },
            [&](int32_t v) { emit s_integerSensorUpdate(qKey, v); },
            [&](uint16_t v) {},
            [&](uint32_t v) {});

        std::visit(museum, valueOpt.value());
    }

} // namespace Kub3::Tools::Tester
