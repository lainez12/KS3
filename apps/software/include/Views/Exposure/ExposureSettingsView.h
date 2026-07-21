#ifndef EXPOSURESETTINGSVIEW_H
#define EXPOSURESETTINGSVIEW_H

#include <QWidget>
#include <ViewModels/Exposure/ExposureSettingsViewModel.h>

#include <Views/ExposureViewBase.h>
#include <Views/KeyboardConnections.h>

namespace Ui
{
    class ExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure
{
    class ExposureSettingsView final : public ExposureViewBase
    {
        using ExposureSettingsViewModel = Kub3::UI::ViewModels::Exposure::ExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit ExposureSettingsView(Unique<ExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~ExposureSettingsView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void onBackButtonClicked();
        void onSaveButtonClicked();
        void onValidateButtonClicked() override;

        void switchToFlashingMode();
        void switchToContinuousMode();

        PresetExposure getCurrentPresetExposure() const;

    private:
        Ui::ExposureSettingsView *ui;
        KeyboardConnections m_keyboard;
        bool m_isFlashingMode = false;
    };

} // namespace Kub3::UI::Views

using ExposureSettingsView = Kub3::UI::Views::ViewsExposure::ExposureSettingsView;

#endif