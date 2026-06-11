#ifndef LOADPARAMETERSVIEW_H
#define LOADPARAMETERSVIEW_H

#include <QWidget>
#include <ViewModels/ViewModelsAlignment/LoadParametersViewModel.h>
#include <Views/AlignmentViewBase.h>

#include "Views/ViewBase.h"

namespace Ui
{
    class LoadParametersView;
} // namespace UI

namespace Kub3::UI::Views::ViewsAlignment
{
    class LoadParametersView final : public AlignmentViewBase
    {
        using LoadParametersViewModel = Kub3::UI::ViewModels::Alignment::LoadParametersViewModel;

        Q_OBJECT

    public:
        explicit LoadParametersView(Unique<LoadParametersViewModel> viewModel, QWidget *parent = nullptr);
        ~LoadParametersView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked(const QString &buttonId) override;
        void onBackButtonClicked(const QString &buttonId) override;

    private:
        Ui::LoadParametersView *ui;
    };

} // namespace Kub3::UI::Views

using LoadParametersView = Kub3::UI::Views::ViewsAlignment::LoadParametersView;

#endif