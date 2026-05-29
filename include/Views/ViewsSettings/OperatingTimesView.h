#ifndef OPERATINGTIMESVIEW_H
#define OPERATINGTIMESVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsSettings/OperatingTimesViewModel.h>
#include <Views/SettingsViewBase.h>

#include "Views/ViewBase.h"

namespace Ui {
    class OperatingTimesView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings {
    class OperatingTimesView final : public SettingsViewBase {
        using OperatingTimesViewModel = Kub3::UI::ViewModels::ViewModelsSettings::OperatingTimesViewModel;

        Q_OBJECT

    public:
        explicit OperatingTimesView(Unique<OperatingTimesViewModel> viewModel, QWidget *parent = nullptr);
        ~OperatingTimesView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::OperatingTimesView *ui;
    };

} // namespace Kub3::UI::Views

using OperatingTimesView = Kub3::UI::Views::ViewsSettings::OperatingTimesView;

#endif