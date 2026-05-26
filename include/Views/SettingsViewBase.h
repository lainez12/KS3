#pragma once

#include <Views/ViewBase.h>

namespace Kub3::UI::Views {

    class SettingsViewBase : public ViewBase {
        Q_OBJECT

    public:
        explicit SettingsViewBase(
            Unique<ViewModels::IViewModel> viewModel,
            QWidget *parent = nullptr);

        virtual ~SettingsViewBase() = default;

    private:
        void createNavButtonsConfigs() final override;
        void configTitleBar() override;

        void onHomeButtonClicked(const QString &buttonId);
        void onBackButtonClicked(const QString &buttonId);

    protected:
        void setDefaultTitleBar(const QString &viewTitle);
    };

} // namespace Kub3::UI::Views