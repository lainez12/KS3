#pragma once

#include <QWidget>
#include <ViewModels/Exposure/SaveExposureSettingsViewModel.h>

#include <Views/ExposureViewBase.h>
#include <Views/KeyboardConnections.h>

namespace Ui
{
    class SaveExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure
{
    class SaveExposureSettingsView final : public ExposureViewBase
    {
        Q_OBJECT

        using SaveExposureSettingsViewModel = Kub3::UI::ViewModels::Exposure::SaveExposureSettingsViewModel;

    public:
        explicit SaveExposureSettingsView(Unique<SaveExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~SaveExposureSettingsView();

    public:
        void setAPresetSavedInThisSession(bool saved);
        bool isAPresetSavedInThisSession() const;
        void userConfirmSaveReplacementPreset(const QString &name);
        // QWidget overrides
        void resizeEvent(QResizeEvent *event) override;
        void showEvent(QShowEvent *event) override;

    public slots:
        void ps_onConfirmButtonClicked();
        void ps_onBackButtonClicked();
        void ps_onPresetSaved();
        void ps_onErrorSavingPreset(const QString &errorMessage);

    private slots:
        void onValidateButtonClicked() override;

    private:
        void setNewNavButtonsConfigs();
        void populateViewWithCurrentPreset();

    private:
        Ui::SaveExposureSettingsView *ui;
        KeyboardConnections m_keyboard;
        bool m_isFlashingMode = false;
        bool m_presetSaved    = true; // true to avoid populating the view with presets when the view is first opened
        QMap<QString, QPushButton *> m_presetsButton;
    };

} // namespace Kub3::UI::Views

using SaveExposureSettingsView = Kub3::UI::Views::ViewsExposure::SaveExposureSettingsView;
