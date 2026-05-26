#ifndef EXPOSURESETTINGSVIEW_H
#define EXPOSURESETTINGSVIEW_H

#include <QWidget>
#include <ViewModels/ExposureSettingsViewModel.h>

#include "ViewBase.h"

namespace Ui {
    class ExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views {
    class ExposureSettingsView final : public ViewBase {
        using ExposureSettingsViewModel = Kub3::UI::ViewModels::ExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit ExposureSettingsView(Unique<ExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~ExposureSettingsView();

    signals:

    public slots:

    private slots:

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void createNavButtonsConfigs();
        void configTitleBar();
        void onHomeButtonClicked(const QString &buttonId);
        void onBackButtonClicked(const QString &buttonId);
        void onSaveButtonClicked(const QString &buttonId);
        void onValidateButtonClicked(const QString &buttonId);

        void switchToFlashingMode();
        void switchToContinuousMode();
    private:
        Ui::ExposureSettingsView *ui;

        bool m_isFlashingMode = false;
    };

} // namespace Kub3::UI::Views

using ExposureSettingsView = Kub3::UI::Views::ExposureSettingsView;

#endif