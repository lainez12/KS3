#pragma once

#include <Views/ViewBase.h>

namespace Kub3::UI::Views
{

    class SettingsViewBase : public ViewBase
    {
        Q_OBJECT

    public:
        explicit SettingsViewBase(Unique<ViewModels::BaseViewModel> viewModel, QWidget *parent = nullptr);
        virtual ~SettingsViewBase() = default;

    protected:
        void onBackButtonClicked();

    private:
        void createNavButtonsConfigs() final override;
        void configTitleBar() override;
        void onHomeButtonClicked();

    protected:
        void setDefaultTitleBar(const QString &viewTitle);
    };

} // namespace Kub3::UI::Views