#ifndef UPDATESOFTWAREVIEW_H
#define UPDATESOFTWAREVIEW_H

#include <QWidget>
#include <ViewModels/Settings/UpdateSoftwareViewModel.h>
#include <Views/SettingsViewBase.h>

#include <Views/ViewBase.h>

namespace Ui
{
    class UpdateSoftwareView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings
{
    class UpdateSoftwareView final : public SettingsViewBase
    {
        using UpdateSoftwareViewModel = Kub3::UI::ViewModels::Settings::UpdateSoftwareViewModel;

        Q_OBJECT

    public:
        explicit UpdateSoftwareView(Unique<UpdateSoftwareViewModel> viewModel, QWidget *parent = nullptr);
        ~UpdateSoftwareView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::UpdateSoftwareView *ui;
    };

} // namespace Kub3::UI::Views

using UpdateSoftwareView = Kub3::UI::Views::ViewsSettings::UpdateSoftwareView;

#endif