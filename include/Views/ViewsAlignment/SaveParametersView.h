#ifndef SAVEPARAMETERSVIEW_H
#define SAVEPARAMETERSVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsAlignment/SaveParametersViewModel.h>
#include <Views/AlignmentViewBase.h>

#include "Views/ViewBase.h"

namespace Ui {
    class SaveParametersView;
} // namespace UI

namespace Kub3::UI::Views::ViewsAlignment {
    class SaveParametersView final : public AlignmentViewBase {
        using SaveParametersViewModel = Kub3::UI::ViewModels::ViewModelsAlignment::SaveParametersViewModel;

        Q_OBJECT

    public:
        explicit SaveParametersView(Unique<SaveParametersViewModel> viewModel, QWidget *parent = nullptr);
        ~SaveParametersView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::SaveParametersView *ui;
    };

} // namespace Kub3::UI::Views

using SaveParametersView = Kub3::UI::Views::ViewsAlignment::SaveParametersView;

#endif