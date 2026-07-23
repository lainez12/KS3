#ifndef SCREENSHOTEXPORTVIEWMODEL_H
#define SCREENSHOTEXPORTVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/BaseViewModel.h>

namespace Kub3::UI::ViewModels::Settings
{
    class ScreenshotExportViewModel final : public BaseViewModel
    {
    public:
        explicit ScreenshotExportViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ScreenshotExportViewModel() override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::Settings

#endif