#include <ViewModels/HomeViewModel.h>

namespace Kub3::UI::ViewModels
{

    HomeViewModel::HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    void HomeViewModel::ps_onSystemStateChanged(MFSM::SystemStateKind state)
    {
        auto prevState = m_currentSysState;

        m_currentSysState = state;
        evaluateLocks();
        evaluatePopup(prevState);
    }

    void HomeViewModel::ps_onOperationalSubstateKindChanged(MFSM::OperationalStateKind state)
    {
        auto prevState = m_currentSysState;

        m_currentOpSubstate = state;
        evaluateLocks();
        evaluatePopup(prevState);
    }

    void HomeViewModel::evaluateLocks()
    {
        // Evaluate rules based on the FSM state
        const bool isWaitingInit = (m_currentSysState == MFSM::SystemStateKind::WaitingInitialization);
        const bool isIdle        = (m_currentSysState == MFSM::SystemStateKind::Operational) &&
                                   (m_currentOpSubstate == MFSM::OperationalStateKind::Idle);

        // Emit instructions to the View
        emit s_setInitializationLock(!isWaitingInit);
        // Everything else requires the machine to be idle (operational) and initialized
        emit s_setDrawerActionsLock(!isIdle);
        emit s_setExposureModeLock(!isIdle);
        emit s_setHomingLock(!isIdle);
    }

    void HomeViewModel::evaluatePopup(MFSM::SystemStateKind prevState)
    {
        using SysKind = MFSM::SystemStateKind;

        if (m_currentSysState == SysKind::Initializing)
        {
            emit s_createPopUpWithText(
                "Initialization",
                {
                    popup_action_t{
                        .text     = "Cancel",
                        .callback = [this]() { this->uiRequestCancel(); },
                    },
                },
                "Starting initialization...");
        }
        else if (prevState == SysKind::Initializing &&
                 (m_currentSysState == SysKind::WaitingInitialization ||
                  m_currentSysState == SysKind::Operational))
        {
            emit s_closePopUp(); // Reacts to abort initialization
        }
    }

    void HomeViewModel::loadConnections(void)
    {
        BaseViewModel::loadConnections();

        // Force an immediate UI sync using the cached FSM state
        evaluateLocks();
    }

    void HomeViewModel::uiRequestCancel()
    {
        emit s_cancelOperation();
    }

    void HomeViewModel::uiRequestInitialization()
    {
        emit s_initializationRequest();
    }

    void HomeViewModel::uiRequestDrawerOperation(DrawerTarget tgt, bool eject)
    {
        const char *tgtStr = (tgt == DrawerTarget::Mask) ? "Mask" : "Wafer";
        const char *opStr  = eject ? "ejection" : "insertion";

        emit s_createPopUpWithText(
            QString("%1 %2 in progress").arg(tgtStr).arg(opStr),
            {
                popup_action_t{
                    .text     = "Cancel",
                    .callback = [this]() { this->uiRequestCancel(); },
                },
            },
            "Preparing operation...");
        emit s_cmdRunDrawerOperation(tgt, eject);
    }

    void HomeViewModel::ps_initializationSuccess()
    {
        emit s_setDrawerActionsLock(false);
        emit s_setExposureModeLock(false);
        emit s_setHomingLock(false);
        emit s_setInitializationLock(true);
    }

    void HomeViewModel::ps_errorOccurred(const MFSM::ErrorPayload &payload)
    {
        if (payload.kind == ErrorKind::Global)
            return;

        // TODO: handle error messages
    }

    void HomeViewModel::ps_operationEnded()
    {
        emit s_closePopUp();
    }

    void HomeViewModel::ps_onProcessMessageBroadcast(const Kub3::Common::ProcessMessage &msg)
    {
        // FILTERING: Home screen only cares about telemetry if it's currently showing a popup for Initialization or a Drawer Operation.
        const bool isInitializing = (m_currentSysState == MFSM::SystemStateKind::Initializing);
        const bool isDrawerOp     = (m_currentSysState == MFSM::SystemStateKind::Operational) &&
                                    (m_currentOpSubstate == MFSM::OperationalStateKind::DrawerOp);

        if (!isInitializing && !isDrawerOp)
        {
            return;
        }

        // FORMATTING
        QString color  = "#1976D2"; // Default blue
        QString prefix = "";

        if (msg.level == Common::ProcessMessageLevel::Error)
        {
            color  = "#F44336";
            prefix = "❌";
        }
        else if (msg.level == Common::ProcessMessageLevel::Warning)
        {
            color  = "#FF9800";
            prefix = "⚠️";
        }
        else if (msg.level == Common::ProcessMessageLevel::Success)
        {
            color  = "#4CAF50";
            prefix = "✔";
        }

        QString html = QString("<div style=\"color:%1; text-align: left;\">%2 %3</div>")
                           .arg(color, prefix, msg.text.toHtmlEscaped());

        // Update the active popup dynamically
        emit s_appendPopUpMessage(html);
    }

}
