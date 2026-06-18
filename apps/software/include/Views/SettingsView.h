#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QWidget>
#include <ViewModels/SettingsViewModel.h>

#include "ViewBase.h"

namespace Ui {
    class SettingsView;
} // namespace UI

namespace Kub3::UI::Views {
    class SettingsView final : public ViewBase {
        using SettingsViewModel = Kub3::UI::ViewModels::SettingsViewModel;

        Q_OBJECT

    public:
        explicit SettingsView(Unique<SettingsViewModel> viewModel, QWidget *parent = nullptr);
        ~SettingsView();

    signals:

    public slots:

    private slots:

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void createNavButtonsConfigs();
        void configTitleBar();
        void onHomeButtonClicked(const QString &buttonId);

    private:
        Ui::SettingsView *ui;
    };

} // namespace Kub3::UI::Views

using SettingsView = Kub3::UI::Views::SettingsView;

#endif