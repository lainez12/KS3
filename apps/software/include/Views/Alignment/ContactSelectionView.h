#ifndef CONTACTSELECTIONVIEW_H
#define CONTACTSELECTIONVIEW_H

#include <QWidget>
#include <ViewModels/Alignment/ContactSelectionViewModel.h>
#include <Views/AlignmentViewBase.h>
#include <Views/KeyboardConnections.h>

#include <Views/ViewBase.h>

namespace Ui
{
    class ContactSelectionView;
} // namespace UI

namespace Kub3::UI::Views
{
    class ContactSelectionView final : public AlignmentViewBase
    {
        using ContactSelectionViewModel = Kub3::UI::ViewModels::Alignment::ContactSelectionViewModel;

        Q_OBJECT

    public:
        explicit ContactSelectionView(Unique<ContactSelectionViewModel> viewModel, QWidget *parent = nullptr);
        ~ContactSelectionView();

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked() override;
        void onBackButtonClicked() override;

    private:
        Ui::ContactSelectionView *ui;
    };

} // namespace Kub3::UI::Views

using ContactSelectionView = Kub3::UI::Views::ContactSelectionView;

#endif