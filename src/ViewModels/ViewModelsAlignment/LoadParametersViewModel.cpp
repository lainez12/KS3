#include <ViewModels/ViewModelsAlignment/LoadParametersViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsAlignment {
    LoadParametersViewModel::LoadParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    LoadParametersViewModel::~LoadParametersViewModel() {
    }

    void LoadParametersViewModel::loadConnections(void) {
    }

    void LoadParametersViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsAlignment