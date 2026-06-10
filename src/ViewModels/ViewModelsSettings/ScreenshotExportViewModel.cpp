#include <ViewModels/ViewModelsSettings/ScreenshotExportViewModel.h>

namespace Kub3::UI::ViewModels::ViewModelsSettings
{
    ScreenshotExportViewModel::ScreenshotExportViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        QObject(parent),
        m_repo(repo)
    {
    }
    ScreenshotExportViewModel::~ScreenshotExportViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::ViewModelsSettings