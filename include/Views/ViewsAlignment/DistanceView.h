#ifndef DISTANCEVIEW_H
#define DISTANCEVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsAlignment/DistanceViewModel.h>
#include <Views/AlignmentViewBase.h>

#include "Views/ViewBase.h"

namespace Ui {
    class DistanceView;
} // namespace UI

namespace Kub3::UI::Views::ViewsAlignment {
    class DistanceView final : public AlignmentViewBase {
        using DistanceViewModel = Kub3::UI::ViewModels::ViewModelsAlignment::DistanceViewModel;

        Q_OBJECT

    public:
        explicit DistanceView(Unique<DistanceViewModel> viewModel, QWidget *parent = nullptr);
        ~DistanceView();

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::DistanceView *ui;
    };

} // namespace Kub3::UI::Views

using DistanceView = Kub3::UI::Views::ViewsAlignment::DistanceView;

#endif