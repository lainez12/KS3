#include <ViewModels/Settings/ScreenshotExportViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    ScreenshotExportViewModel::ScreenshotExportViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent) :
        BaseViewModel(parent),
        m_repo(repo)
    {
    }
    ScreenshotExportViewModel::~ScreenshotExportViewModel()
    {
    }

} // namespace Kub3::UI::ViewModels::Settings