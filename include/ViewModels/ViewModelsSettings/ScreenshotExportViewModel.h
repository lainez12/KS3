#ifndef SCREENSHOTEXPORTVIEWMODEL_H
#define SCREENSHOTEXPORTVIEWMODEL_H

#include <HAL/MachineStatus/IMachineStatusRepo.h>
#include <QObject>
#include <ViewModels/IViewModel.h>

namespace Kub3::UI::ViewModels::ViewsModelSettings {
    class ScreenshotExportViewModel final : public QObject, public IViewModel {
        Q_OBJECT
    public:
        explicit ScreenshotExportViewModel(Shared<HAL::MS::IMachineStatusRepo> repo, QObject *parent = nullptr);
        ~ScreenshotExportViewModel() override;

        void loadConnections(void) override;
        void unloadConnections(void) override;

    private:
        Shared<HAL::MS::IMachineStatusRepo> m_repo;
    };

} // namespace Kub3::UI::ViewModels::ViewsModelSettings

#endif