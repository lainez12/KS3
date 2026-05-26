#ifndef SCREENSHOTEXPORTVIEW_H
#define SCREENSHOTEXPORTVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsSettings/ScreenshotExportViewModel.h>
#include <Views/SettingsViewBase.h>

#include "Views/ViewBase.h"

namespace Ui {
    class ScreenshotExportView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings {
    class ScreenshotExportView final : public SettingsViewBase {
        using ScreenshotExportViewModel = Kub3::UI::ViewModels::ViewsModelSettings::ScreenshotExportViewModel;

        Q_OBJECT

    public:
        explicit ScreenshotExportView(Unique<ScreenshotExportViewModel> viewModel, QWidget *parent = nullptr);
        ~ScreenshotExportView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::ScreenshotExportView *ui;
    };

} // namespace Kub3::UI::Views

using ScreenshotExportView = Kub3::UI::Views::ViewsSettings::ScreenshotExportView;

#endif