#pragma once

#include <QWidget>
#include <ViewModels/Alignment/RenameParametersViewModel.h>
#include <Views/AlignmentViewBase.h>
#include <Views/Components/DoubleClickButton.h>

#include <Views/KeyboardConnections.h>
#include <Views/ViewBase.h>

namespace Ui
{
    class RenameParametersView;
} // namespace UI

namespace Kub3::UI::Views
{
    using RenameParametersViewModel = Kub3::UI::ViewModels::Alignment::RenameParametersViewModel;

    class RenameParametersView final : public AlignmentViewBase
    {
        Q_OBJECT

    public:
        explicit RenameParametersView(Unique<RenameParametersViewModel> viewModel, QWidget *parent = nullptr);
        ~RenameParametersView();

    public:
        // QWidget overrides
        void resizeEvent(QResizeEvent *event) override;
        void showEvent(QShowEvent *event) override;

    public slots:
        void ps_onErrorRenamingParameter(const QString &errorMessage);
        void ps_onParameterRenamed();

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked() override;
        void onConfirmButtonClicked();
        void onBackButtonClicked() override;

    private:
        Ui::RenameParametersView *ui;
        KeyboardConnections m_keyboard;
        bool m_parameterSaved = true; // true to avoid populating the view with parameters when the view is first opened
        QMap<QString, QPushButton *> m_presetsButton;
    };

} // namespace Kub3::UI::Views

using RenameParametersView = Kub3::UI::Views::RenameParametersView;
