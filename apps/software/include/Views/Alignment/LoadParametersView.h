#ifndef LOADPARAMETERSVIEW_H
#define LOADPARAMETERSVIEW_H

#include <QCheckBox>
#include <QWidget>
#include <ViewModels/Alignment/AlignmentParametersPersistence.h>
#include <ViewModels/Alignment/LoadParametersViewModel.h>
#include <Views/AlignmentViewBase.h>

#include <Views/ViewBase.h>

namespace Ui
{
    class LoadParametersView;
} // namespace UI

namespace Kub3::UI::Views
{
    class LoadParametersView final : public AlignmentViewBase
    {
        using alignment_parameter_t   = ViewModels::Alignment::Persistence::alignment_parameter_t;
        using LoadParametersViewModel = Kub3::UI::ViewModels::Alignment::LoadParametersViewModel;

        Q_OBJECT

    public:
        explicit LoadParametersView(Unique<LoadParametersViewModel> viewModel, QWidget *parent = nullptr);
        ~LoadParametersView();

    public:
        void resizeEvent(QResizeEvent *event) override;
        void showEvent(QShowEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked() override;
        void onBackButtonClicked() override;
        void populateParametersList();

    private:
        Ui::LoadParametersView *ui;
        QMap<QString, QCheckBox *> m_parametersButtons;
        QMap<QString, QCheckBox *> m_parametersDeleteButtons;
    };

} // namespace Kub3::UI::Views

using LoadParametersView = Kub3::UI::Views::LoadParametersView;

#endif