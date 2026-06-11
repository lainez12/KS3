#ifndef DISTANCEVIEW_H
#define DISTANCEVIEW_H

#include <QWidget>
#include <ViewModels/Alignment/DistanceViewModel.h>
#include <Views/AlignmentViewBase.h>
#include <Views/KeyboardConnections.h>

#include <Views/ViewBase.h>

namespace Ui
{
    class DistanceView;
} // namespace UI

namespace Kub3::UI::Views::ViewsAlignment
{
    class DistanceView final : public AlignmentViewBase
    {
        using DistanceViewModel = Kub3::UI::ViewModels::Alignment::DistanceViewModel;

        Q_OBJECT

    public:
        explicit DistanceView(Unique<DistanceViewModel> viewModel, QWidget *parent = nullptr);
        ~DistanceView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked(const QString &buttonId) override;
        void onBackButtonClicked(const QString &buttonId) override;

    private:
        Ui::DistanceView *ui;
        KeyboardConnections m_keyboard;
    };

} // namespace Kub3::UI::Views

using DistanceView = Kub3::UI::Views::ViewsAlignment::DistanceView;

#endif