#ifndef SAVEEXPOSURESETTINGSVIEW_H
#define SAVEEXPOSURESETTINGSVIEW_H

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
        using SaveExposureSettingsViewModel = Kub3::UI::ViewModels::Exposure::SaveExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit SaveExposureSettingsView(Unique<SaveExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~SaveExposureSettingsView();

    public:
        void resizeEvent(QResizeEvent *event) override;
        void showEvent(QShowEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onBackButtonClicked();
        void onValidateButtonClicked() override;

        void populateViewWithCurrentPreset();

    public slots:
        void onConfirmButtonClicked();
        void s_onPresetSaved();
        void s_onErrorSavingPreset(const QString &errorMessage);
        void setAPresetSavedInThisSession(bool saved);
        bool isAPresetSavedInThisSession() const;
        void userConfirmSaveReplacementPreset(const QString &name);

    private:
        Ui::SaveExposureSettingsView *ui;
        KeyboardConnections m_keyboard;
        bool m_isFlashingMode = false;
        bool m_PresetSaved    = true; // true to avoid populating the view with presets when the view is first opened
        QMap<QString, QPushButton *> m_presetsButton;
    };

} // namespace Kub3::UI::Views

using SaveExposureSettingsView = Kub3::UI::Views::ViewsExposure::SaveExposureSettingsView;

#endif