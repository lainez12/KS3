#include <HAL/MachineStatus/sensors_labels.h>
#include <HAL/MachineStatus/utils.h>
#include <HAL/MachineStatus/virtual_labels.h>
#include <ViewModels/ExposureMenuViewModel.h>

namespace Kub3::UI::ViewModels
{

    ExposureMenuViewModel::ExposureMenuViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    void ExposureMenuViewModel::loadConnections()
    {
        BaseViewModel::loadConnections();

        updateView();
    }

    // ==========================================
    // FSM Listeners
    // ==========================================

    void ExposureMenuViewModel::ps_onSystemStateChanged(MFSM::SystemStateKind state)
    {
        m_currentSysState = state;
        updateView();
    }

    void ExposureMenuViewModel::ps_onOperationalSubstateChanged(const MFSM::OperationalState &state)
    {
        const bool wasIdle   = std::holds_alternative<MFSM::StateIdle>(m_currentOpSubstate);
        const bool isNowIdle = std::holds_alternative<MFSM::StateIdle>(state);

        if (wasIdle && !isNowIdle)
        {
            if (const auto *stowing = std::get_if<MFSM::StateStowing>(&state))
            {
                if (has_flag(stowing->target, StowageTarget::Mask))
                    emit s_clearMaskSequenceLogs();

                if (has_flag(stowing->target, StowageTarget::Wafer))
                    emit s_clearWaferSequenceLogs();
            }
            else if (std::holds_alternative<MFSM::StateAutoleveling>(state))
            {
                emit s_clearAutolevelSequenceLogs();
            }
        }

        m_currentOpSubstate = state;
        updateView();
    }

    void ExposureMenuViewModel::ps_onPostureChanged(const MFSM::SystemPosture &posture)
    {
        m_machinePosture = posture;
        updateView();
    }

    void ExposureMenuViewModel::ps_onProcessMessageBroadcast(const Kub3::Common::ProcessMessage &msg)
    {
        const bool isRelevantOperation = std::holds_alternative<MFSM::StateStowing>(m_currentOpSubstate) ||
                                         std::holds_alternative<MFSM::StateAutoleveling>(m_currentOpSubstate);

        if (!isRelevantOperation)
            return;

        QString text = msg.text.toHtmlEscaped();
        QString color;
        QString prefix;

        switch (msg.level)
        {
        case Common::ProcessMessageLevel::Info:
            color  = "#A0A0A0";
            prefix = "➤ ";
            break;
        case Common::ProcessMessageLevel::Success:
            color  = "#4CAF50";
            prefix = "✔ ";
            break;
        case Common::ProcessMessageLevel::Warning:
            color  = "#FFC107";
            prefix = "⚠ ";
            break;
        case Common::ProcessMessageLevel::Error:
            color  = "#F44336";
            prefix = "✖ ";
            break;
        }

        if (msg.level == Common::ProcessMessageLevel::Error)
        {
            text = "<b>" + text + "</b>";
        }

        QString html     = QString("<span style=\"color:%1;\">%2 %3</span>").arg(color, prefix, text);
        Operation currOp = currentOperation();

        if (has_flag(currOp, Operation::STOWING_MASK))
            emit s_appendMaskSequenceLog(html);
        if (has_flag(currOp, Operation::STOWING_WAFER))
            emit s_appendWaferSequenceLog(html);
        if (has_flag(currOp, Operation::AUTOLEVELING))
            emit s_appendAutolevelSequenceLog(html);
    }

    // ==========================================
    // From the UI
    // ==========================================

    void ExposureMenuViewModel::ui_requestLoadMaskToExposure()
    {
        emit s_cmdOperateStowage(StowageTarget::Mask);
    }

    void ExposureMenuViewModel::ui_requestLoadWaferToExposure()
    {
        if (m_machinePosture.mask != MFSM::MaskPosture::Exposure)
        {
            emit s_cmdOperateStowage(StowageTarget::Both);
        }
        else
        {
            emit s_cmdOperateStowage(StowageTarget::Wafer);
        }
    }

    void ExposureMenuViewModel::ui_requestAutoleveling()
    {
        emit s_cmdStartAutolevel();
    }

    void ExposureMenuViewModel::ui_requestCancel()
    {
        emit s_cmdCancelOperation();
    }

    // ==========================================
    // Core Logic Evaluator
    // ==========================================

    void ExposureMenuViewModel::updateView()
    {
        // Base check of the operational state
        const bool isOperational = (m_currentSysState == MFSM::SystemStateKind::Operational);

        if (!isOperational)
        {
            emit s_setAllLocks(true);
            return;
        }

        // Extract Physical Postures
        const bool isMaskStowed           = (m_machinePosture.mask == MFSM::MaskPosture::Exposure);
        const bool isWaferInAlignmentZone = (m_machinePosture.wafer == MFSM::WaferPosture::AlignmentZone);
        const bool isLevelingValid        = m_machinePosture.isLevelingValid;
        // Query repo for data (Hardware present & Saved data)
        const bool canStowMask           = !HAL::MS::readBool(m_repo, CM3);
        const bool hasSavedHorizontality = HAL::MS::readBool(m_repo, V_HORIZONTALITY_SAVED);
        // Get currently running operation
        const bool isIdle      = std::holds_alternative<MFSM::StateIdle>(m_currentOpSubstate);
        const Operation currOp = isIdle ? Operation::NONE : currentOperation();

        // ==========================================
        // Update the checkmarks
        // ==========================================
        StepStatus maskStatus = StepStatus::NotStarted;
        if (has_flag(currOp, Operation::STOWING_MASK))
            maskStatus = StepStatus::Running;
        else if (isMaskStowed)
            maskStatus = StepStatus::Success; // (Or Homed, if !canStowMask)

        StepStatus waferStatus = StepStatus::NotStarted;
        if (has_flag(currOp, Operation::STOWING_WAFER))
            waferStatus = StepStatus::Running;
        else if (isWaferInAlignmentZone)
            waferStatus = StepStatus::Success;

        StepStatus alStatus = StepStatus::NotStarted;
        if (has_flag(currOp, Operation::AUTOLEVELING))
            alStatus = StepStatus::Running;
        else if (isLevelingValid)
            alStatus = StepStatus::Success;

        emit s_setMaskStatus(maskStatus);
        emit s_setWaferStatus(waferStatus);
        emit s_setAutolevelStatus(alStatus);

        // ==========================================
        // Update the buttons' locks
        // ==========================================

        // Load Mask: Allowed if idle, operational, and mask is not already stowed
        emit s_setMaskLoadLock(!isIdle || isMaskStowed);
        emit s_setMaskCancelLock(currOp != Operation::STOWING_MASK); // Unlock ONLY if mask is actively stowing
        // Load Wafer: Allowed if idle, mask is stowed (interlock rule), and wafer is not up
        emit s_setWaferLoadLock(!isIdle || !isMaskStowed || isWaferInAlignmentZone);
        emit s_setWaferCancelLock(currOp != Operation::STOWING_WAFER); // Unlock ONLY if wafer is actively stowing
        // Autolevel: Allowed if idle, mask stowed, and wafer in alignment zone
        emit s_setAutolevelLock(!isIdle || !isMaskStowed || !isWaferInAlignmentZone);
        emit s_setAutolevelCancelLock(currOp != Operation::AUTOLEVELING);
        // Horizontality: allow saving if possible and loading if available
        emit s_setSaveHorizontalityLock(!isIdle || !isLevelingValid);
        emit s_setLoadHorizontalityLock(!isIdle || !hasSavedHorizontality || !isMaskStowed || !isWaferInAlignmentZone);

        // ==========================================
        // Update the continue to exposure button
        // ==========================================
        const bool isMaskReady = canStowMask ? isMaskStowed : (m_machinePosture.mask == MFSM::MaskPosture::Homed);
        const bool canContinue = isMaskReady && isWaferInAlignmentZone;

        qDebug() << "Is mask ready:" << isMaskReady << ", isWaferInAlignmentZone:" << isWaferInAlignmentZone;

        emit s_setContinueToExposureLock(!canContinue);
    }

    Operation ExposureMenuViewModel::currentOperation() const
    {
        if (m_currentSysState != MFSM::SystemStateKind::Operational)
            return Operation::NONE;

        Operation ret = Operation::NONE;

        // Extract exact Granular Operation from the Variant
        if (const auto *stowing = std::get_if<MFSM::StateStowing>(&m_currentOpSubstate))
        {
            if ((stowing->target & StowageTarget::Mask) != StowageTarget::None)
                ret = ret | Operation::STOWING_MASK;
            if ((stowing->target & StowageTarget::Wafer) != StowageTarget::None)
                ret = ret | Operation::STOWING_WAFER;
        }
        else if (const auto *alignment = std::get_if<MFSM::StateAlignment>(&m_currentOpSubstate))
        {
            if (alignment->phase == MFSM::ContactPhase::ApplyingContact)
                ret = ret | Operation::AUTOLEVELING;
        }

        return ret;
    }

}