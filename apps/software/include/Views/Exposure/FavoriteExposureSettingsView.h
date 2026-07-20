#ifndef FAVORITEEXPOSURESETTINGSVIEW_H
#define FAVORITEEXPOSURESETTINGSVIEW_H

#include <QList>
#include <QWidget>
#include <ViewModels/Exposure/FavoriteExposureSettingsViewModel.h>

class FavoriteExposureSettingButton;

#include <Views/ExposureViewBase.h>

namespace Ui
{
    class FavoriteExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure
{
    class FavoriteExposureSettingsView final : public ExposureViewBase
    {
        using FavoriteExposureSettingsViewModel = Kub3::UI::ViewModels::Exposure::FavoriteExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit FavoriteExposureSettingsView(Unique<FavoriteExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~FavoriteExposureSettingsView();

    private:
        void setNewNavButtonsConfigs();
        void populateStackedFavorite(const QList<FavoriteExposureSettingButton *> &favoriteButtons);
        void updateFavoritePageNavigation();
        void onBackButtonClicked();
        void onValidateButtonClicked() override;

    private:
        Ui::FavoriteExposureSettingsView *ui;
    };

} // namespace Kub3::UI::Views

using FavoriteExposureSettingsView = Kub3::UI::Views::ViewsExposure::FavoriteExposureSettingsView;

#endif