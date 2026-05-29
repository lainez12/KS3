#ifndef EXPOSURESETTINGSVIEW_H
#define EXPOSURESETTINGSVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsExposure/ExposureSettingsViewModel.h>

#include <Views/ExposureViewBase.h>

namespace Ui {
    class ExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure {
    class ExposureSettingsView final : public ExposureViewBase {
        using ExposureSettingsViewModel = Kub3::UI::ViewModels::ViewsModelsExposure::ExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit ExposureSettingsView(Unique<ExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~ExposureSettingsView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onBackButtonClicked(const QString &buttonId);
        void onSaveButtonClicked(const QString &buttonId);
        void onValidateButtonClicked(const QString &buttonId) override;

        void switchToFlashingMode();
        void switchToContinuousMode();

    private:
        Ui::ExposureSettingsView *ui;

        bool m_isFlashingMode = false;
    };

} // namespace Kub3::UI::Views

using ExposureSettingsView = Kub3::UI::Views::ViewsExposure::ExposureSettingsView;

#endif