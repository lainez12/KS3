#include <ViewModels/Alignment/ContactSelectionViewModel.h>

namespace Kub3::UI::ViewModels::Alignment
{

    ContactSelectionViewModel::ContactSelectionViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }

    ContactSelectionViewModel::~ContactSelectionViewModel()
    {
    }

    void ContactSelectionViewModel::ui_setSelectedContactMode(ContactMode mode)
    {
        emit s_contactModeChanged(mode);
    }

} // namespace Kub3::UI::ViewModels::Alignment