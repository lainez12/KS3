#ifndef FAVORITEEXPOSURESETTINGSVIEW_H
#define FAVORITEEXPOSURESETTINGSVIEW_H

#include <QWidget>
#include <ViewModels/FavoriteExposureSettingsViewModel.h>

#include "ViewBase.h"

namespace Ui {
    class FavoriteExposureSettingsView;
} // namespace UI

namespace Kub3::UI::Views {
    class FavoriteExposureSettingsView final : public ViewBase {
        using FavoriteExposureSettingsViewModel = Kub3::UI::ViewModels::FavoriteExposureSettingsViewModel;

        Q_OBJECT

    public:
        explicit FavoriteExposureSettingsView(Unique<FavoriteExposureSettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~FavoriteExposureSettingsView();

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

    private:
        Ui::FavoriteExposureSettingsView *ui;

        bool m_isFlashingMode = false;
    };

} // namespace Kub3::UI::Views

using FavoriteExposureSettingsView = Kub3::UI::Views::FavoriteExposureSettingsView;

#endif