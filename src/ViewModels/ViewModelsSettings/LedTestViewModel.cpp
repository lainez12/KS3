#include <ViewModels/ViewModelsSettings/LedTestViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    LedTestViewModel::LedTestViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    LedTestViewModel::~LedTestViewModel() {
    }

    void LedTestViewModel::loadConnections(void) {
    }

    void LedTestViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewsModelSettings