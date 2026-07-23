#include <QThread>

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <ViewModels/ProcedureTestViewModel.h>

namespace Kub3::Tools::Tester
{

    ProcedureTestViewModel::ProcedureTestViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        UI::ViewModels::BaseViewModel(parent),
        m_repo(std::move(repo))
    {
    }

    Optional<uint16_t> ProcedureTestViewModel::getADCTareValue(const QString &tareId) const noexcept
    {
        return m_repo->getValue<uint16_t>(tareId.toStdString());
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

    void ProcedureTestViewModel::uiRequestInitStages(void)
    {
        if (!m_isRunning)
            emit cmdRunInitStages();
    }

    void ProcedureTestViewModel::uiRequestCenterStages(void)
    {
        if (!m_isRunning)
            emit cmdRunCenterStages();
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

    void ProcedureTestViewModel::uiRequestInitDrawer(DrawerTarget tgt)
    {
        if (!m_isRunning)
            emit cmdRunInitDrawer(tgt);
    }

    void ProcedureTestViewModel::uiRequestHomeDrawer(DrawerTarget tgt)
    {
        if (!m_isRunning)
            emit cmdRunHomeDrawer(tgt);
    }

    void ProcedureTestViewModel::uiRequestInitZAxes(void)
    {
        if (!m_isRunning)
            emit cmdRunInitZAxes();
    }

    void ProcedureTestViewModel::uiRequestHomeZAxes(void)
    {
        if (!m_isRunning)
            emit cmdRunHomeZAxes();
    }

    void ProcedureTestViewModel::uiRequestAutolevel(void)
    {
        if (!m_isRunning)
            emit cmdRunAutolevel();
    }

    void ProcedureTestViewModel::uiRequestForceSensorTare(ForceSensor fs)
    {
        if (!m_isRunning)
            emit cmdForceSensorTare(fs);
    }

    void ProcedureTestViewModel::uiRequestForceSensorsToggle(bool en)
    {
        emit cmdForceSensorsToggle(en);
    }

    void ProcedureTestViewModel::uiRequestCameraMovement(CameraId camId, MovementKind kind, CameraDirection dir)
    {
        if (m_isRunning)
            return;

        if (kind == MovementKind::CONTINUOUS)
            m_activeContinuousCameraMoves[camId] = true;
        else if (kind == MovementKind::STOP)
        {
            auto it = m_activeContinuousCameraMoves.find(camId);

            if (it == m_activeContinuousCameraMoves.end() || !m_activeContinuousCameraMoves.at(camId))
                return; // Block stop signal here if movement was not continuous
            m_activeContinuousCameraMoves[camId] = false;
        }

        // Send request for movement
        emit cmdRunCameraMovement(camId, kind, dir);
    }

    void ProcedureTestViewModel::uiRequestAlignmentStageMovement(
        AlignmentStageId stageId,
        MovementKind kind,
        AlignmentStageDirection dir)
    {
        if (m_isRunning)
            return;

        if (kind == MovementKind::CONTINUOUS)
            m_activeContinuousAlignmentStageMoves[stageId] = true;
        else if (kind == MovementKind::STOP)
        {
            auto it = m_activeContinuousAlignmentStageMoves.find(stageId);

            if (it == m_activeContinuousAlignmentStageMoves.end() || !m_activeContinuousAlignmentStageMoves.at(stageId))
                return; // Block stop signal here if movement was not continuous
            m_activeContinuousAlignmentStageMoves[stageId] = false;
        }

        // Send request for movement
        emit cmdRunAlignmentStageMovement(stageId, kind, dir);
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

    void ProcedureTestViewModel::onMachineValueChanged(const std::string &key)
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
            [&](uint16_t v) { emit s_uint16SensorUpdate(qKey, v); },
            [&](uint32_t v) {},
            [&](double v) {});

        std::visit(museum, valueOpt.value());
    }

} // namespace Kub3::Tools::Tester
