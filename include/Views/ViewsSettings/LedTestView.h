#ifndef LEDTESTVIEW_H
#define LEDTESTVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsSettings/LedTestViewModel.h>
#include <Views/SettingsViewBase.h>

#include "Views/ViewBase.h"

namespace Ui {
    class LedTestView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings {
    class LedTestView final : public SettingsViewBase {
        using LedTestViewModel = Kub3::UI::ViewModels::ViewsModelSettings::LedTestViewModel;

        Q_OBJECT

    public:
        explicit LedTestView(Unique<LedTestViewModel> viewModel, QWidget *parent = nullptr);
        ~LedTestView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::LedTestView *ui;
    };

} // namespace Kub3::UI::Views

using LedTestView = Kub3::UI::Views::ViewsSettings::LedTestView;

#endif