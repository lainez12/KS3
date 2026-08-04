#ifndef SAVEPARAMETERSVIEW_H
#define SAVEPARAMETERSVIEW_H

#include <QWidget>
#include <ViewModels/Alignment/SaveParametersViewModel.h>
#include <Views/AlignmentViewBase.h>

#include <Views/KeyboardConnections.h>
#include <Views/ViewBase.h>

namespace Ui
{
    class SaveParametersView;
} // namespace UI

namespace Kub3::UI::Views
{
    using SaveParametersViewModel = Kub3::UI::ViewModels::Alignment::SaveParametersViewModel;

    class SaveParametersView final : public AlignmentViewBase
    {
        Q_OBJECT

    public:
        explicit SaveParametersView(Unique<SaveParametersViewModel> viewModel, QWidget *parent = nullptr);
        ~SaveParametersView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked() override;
        void onBackButtonClicked() override;

    private:
        Ui::SaveParametersView *ui;
        KeyboardConnections m_keyboard;
    };

} // namespace Kub3::UI::Views

using SaveParametersView = Kub3::UI::Views::SaveParametersView;

#endif