#ifndef FAVORITEEXPOSURESETTINGSVIEW_H
#define FAVORITEEXPOSURESETTINGSVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsExposure/FavoriteExposureSettingsViewModel.h>

#include <Views/ExposureViewBase.h>

namespace Ui {
    class FavoriteExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure {
    class FavoriteExposureSettingsView final : public ExposureViewBase {
        using FavoriteExposureSettingsViewModel = Kub3::UI::ViewModels::ViewModelsExposure::FavoriteExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit FavoriteExposureSettingsView(Unique<FavoriteExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~FavoriteExposureSettingsView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onBackButtonClicked(const QString &buttonId);
        void onValidateButtonClicked(const QString &buttonId) override;

    private:
        Ui::FavoriteExposureSettingsView *ui;
    };

} // namespace Kub3::UI::Views

using FavoriteExposureSettingsView = Kub3::UI::Views::ViewsExposure::FavoriteExposureSettingsView;

#endif