#include <ViewModels/ViewModelsAlignment/SaveParametersViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsAlignment {
    SaveParametersViewModel::SaveParametersViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    SaveParametersViewModel::~SaveParametersViewModel() {
    }

    void SaveParametersViewModel::loadConnections(void) {
    }

    void SaveParametersViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsAlignment