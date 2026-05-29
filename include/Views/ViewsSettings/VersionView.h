#ifndef VERSIONVIEW_H
#define VERSIONVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsSettings/VersionViewModel.h>
#include <Views/SettingsViewBase.h>

#include "Views/ViewBase.h"

namespace Ui {
    class VersionView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings {
    class VersionView final : public SettingsViewBase {
        using VersionViewModel = Kub3::UI::ViewModels::ViewModelsSettings::VersionViewModel;

        Q_OBJECT

    public:
        explicit VersionView(Unique<VersionViewModel> viewModel, QWidget *parent = nullptr);
        ~VersionView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::VersionView *ui;
    };

} // namespace Kub3::UI::Views

using VersionView = Kub3::UI::Views::ViewsSettings::VersionView;

#endif