#include <ViewModels/ViewModelsSettings/ScreenshotExportViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    ScreenshotExportViewModel::ScreenshotExportViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo) {
    }
    ScreenshotExportViewModel::~ScreenshotExportViewModel() {
    }

    void ScreenshotExportViewModel::loadConnections(void) {
    }

    void ScreenshotExportViewModel::unloadConnections(void) {
    }

} // namespace Kub3::UI::ViewModels::ViewsModelSettings