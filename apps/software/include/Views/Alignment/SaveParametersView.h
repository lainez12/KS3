#pragma once

#include <QWidget>
#include <ViewModels/Alignment/SaveParametersViewModel.h>
#include <Views/AlignmentViewBase.h>
#include <Views/Components/DoubleClickButton.h>

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
        void userConfirmSaveReplacementParameter(const QString &name);
        // QWidget overrides
        void resizeEvent(QResizeEvent *event) override;
        void showEvent(QShowEvent *event) override;

    public slots:
        /// void ps_onConfirmButtonClicked();

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked() override;
        void onBackButtonClicked() override;
        void setAParameterSavedInThisSession(bool saved);
        bool isAParameterSavedInThisSession() const;
        void populateViewWithCurrentParameter();

    private:
        Ui::SaveParametersView *ui;
        KeyboardConnections m_keyboard;
        bool m_parameterSaved = true; // true to avoid populating the view with parameters when the view is first opened
        QMap<QString, QPushButton *> m_presetsButton;
    };

} // namespace Kub3::UI::Views

using SaveParametersView = Kub3::UI::Views::SaveParametersView;

