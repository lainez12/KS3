#ifndef VISUALISATIONVIEW_H
#define VISUALISATIONVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsAlignment/VisualisationViewModel.h>
#include <Views/AlignmentViewBase.h>

#include "Views/ViewBase.h"

namespace Ui {
    class VisualisationView;
} // namespace UI

namespace Kub3::UI::Views::ViewsAlignment {
    class VisualisationView final : public AlignmentViewBase {
        using VisualisationViewModel = Kub3::UI::ViewModels::ViewModelsAlignment::VisualisationViewModel;

        Q_OBJECT

    public:
        explicit VisualisationView(Unique<VisualisationViewModel> viewModel, QWidget *parent = nullptr);
        ~VisualisationView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::VisualisationView *ui;
    };

} // namespace Kub3::UI::Views

using VisualisationView = Kub3::UI::Views::ViewsAlignment::VisualisationView;

#endif