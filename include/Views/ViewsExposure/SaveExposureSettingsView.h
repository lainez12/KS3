#ifndef SAVEEXPOSURESETTINGSVIEW_H
#define SAVEEXPOSURESETTINGSVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsExposure/SaveExposureSettingsViewModel.h>

#include <Views/ExposureViewBase.h>
#include <Views/KeyboardConnections.h>

namespace Ui {
    class SaveExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure {
    class SaveExposureSettingsView final : public ExposureViewBase {
        using SaveExposureSettingsViewModel = Kub3::UI::ViewModels::ViewModelsExposure::SaveExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit SaveExposureSettingsView(Unique<SaveExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~SaveExposureSettingsView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onBackButtonClicked(const QString &buttonId);
        void onSaveButtonClicked(const QString &buttonId);
        void onValidateButtonClicked(const QString &buttonId) override;

    private:
        Ui::SaveExposureSettingsView *ui;
        KeyboardConnections m_keyboard;
        bool m_isFlashingMode = false;
    };

} // namespace Kub3::UI::Views

using SaveExposureSettingsView = Kub3::UI::Views::ViewsExposure::SaveExposureSettingsView;

#endif