#ifndef RECAPEXPOSURESETTINGSVIEW_H
#define RECAPEXPOSURESETTINGSVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsExposure/RecapExposureSettingsViewModel.h>

#include <Views/ExposureViewBase.h>

namespace Ui {
    class RecapExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure {
    class RecapExposureSettingsView final : public ExposureViewBase {
        using RecapExposureSettingsViewModel = Kub3::UI::ViewModels::ViewModelsExposure::RecapExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit RecapExposureSettingsView(Unique<RecapExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~RecapExposureSettingsView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onBackButtonClicked(const QString &buttonId);
        void onValidateButtonClicked(const QString &buttonId) override;

    private:
        Ui::RecapExposureSettingsView *ui;
    };

} // namespace Kub3::UI::Views

using RecapExposureSettingsView = Kub3::UI::Views::ViewsExposure::RecapExposureSettingsView;

#endif