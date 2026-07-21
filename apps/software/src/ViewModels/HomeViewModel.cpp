#include <ViewModels/HomeViewModel.h>

namespace Kub3::UI::ViewModels
{

    HomeViewModel::HomeViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }

    HomeViewModel::~HomeViewModel()
    {
    }

    void HomeViewModel::uiRequestInitialization()
    {
        emit s_initializationRequest();
    }

    void HomeViewModel::uiRequestDrawerInsertion(DrawerTarget tgt) {}

    void HomeViewModel::uiRequestDrawerEjection(DrawerTarget tgt) {}

    void HomeViewModel::ps_errorOccurred(const MFSM::ErrorPayload &payload)
    {
        if (payload.kind == ErrorKind::Global)
            return;

        // TODO: handle error message
    }

}
