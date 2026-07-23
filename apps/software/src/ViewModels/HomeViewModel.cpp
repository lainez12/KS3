#include <ViewModels/HomeViewModel.h>

namespace Kub3::UI::ViewModels
{

    HomeViewModel::HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    HomeViewModel::~HomeViewModel()
    {
    }

    void HomeViewModel::uiRequestCancel()
    {
        emit s_cancelOperation();
    }

    void HomeViewModel::uiRequestInitialization()
    {
        emit s_createPopUpWithText(
            "Initialization",
            {
                popup_action_t{
                    .text     = "Cancel",
                    .callback = [this]() { this->uiRequestCancel(); },
                },
            },
            "Initialization in progress");
        emit s_initializationRequest();
    }

    void HomeViewModel::uiRequestDrawerOperation(DrawerTarget tgt, bool eject)
    {
        const char *tgtStr = (tgt == DrawerTarget::Mask) ? "Mask" : "Wafer";
        const char *opStr  = eject ? "ejection" : "insertion";

        emit s_createPopUp(
            QString("%1 %2 in progress").arg(tgtStr).arg(opStr),
            {
                popup_action_t{
                    .text     = "Cancel",
                    .callback = [this]() { this->uiRequestCancel(); },
                },
            });
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

        // TODO: handle error message
    }

    void HomeViewModel::ps_operationEnded()
    {
        emit s_closePopUp();
    }

}
