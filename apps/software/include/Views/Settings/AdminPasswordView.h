#pragma once

#include <QWidget>
#include <ViewModels/Settings/AdminPasswordViewModel.h>
#include <Views/SettingsViewBase.h>

#include <Views/KeyboardConnections.h>
#include <Views/ViewBase.h>

namespace Ui
{
    class AdminPasswordView;
} // namespace UI

namespace Kub3::UI::Views::ViewsSettings
{
    using AdminPasswordViewModel = Kub3::UI::ViewModels::Settings::AdminPasswordViewModel;

    class AdminPasswordView final : public SettingsViewBase
    {
        Q_OBJECT

    public:
        explicit AdminPasswordView(Unique<AdminPasswordViewModel> viewModel, QWidget *parent = nullptr);
        ~AdminPasswordView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void onAuthenticationFailureFeedback(void);

    private:
        void setNewNavButtonsConfigs();
        void onValidateButtonClicked(void);

    private:
        Ui::AdminPasswordView *ui;
        KeyboardConnections m_keyboard;
    };

} // namespace Kub3::UI::Views

using AdminPasswordView = Kub3::UI::Views::ViewsSettings::AdminPasswordView;
