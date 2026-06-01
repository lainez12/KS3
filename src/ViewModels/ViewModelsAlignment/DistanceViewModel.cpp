#include <ViewModels/ViewModelsAlignment/DistanceViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsAlignment {
    DistanceViewModel::DistanceViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    DistanceViewModel::~DistanceViewModel() {
    }

    void DistanceViewModel::loadConnections(void) {
    }

    void DistanceViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsAlignment