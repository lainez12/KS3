#pragma once

#include <Views/ViewBase.h>

namespace Kub3::UI::Views {

    class AlignmentViewBase : public ViewBase {
        Q_OBJECT

    public:
        explicit AlignmentViewBase(
            Unique<ViewModels::IViewModel> viewModel,
            QWidget *parent = nullptr);

        virtual ~AlignmentViewBase() = default;

    private:
        void configTitleBar() override;

    protected:
        void setDefaultTitleBar(const QString &viewTitle);
        void onHomeButtonClicked(const QString &buttonId);
        void createNavButtonsConfigs() final override;
        virtual void onValidateButtonClicked(const QString &buttonId) = 0;
    };

} // namespace Kub3::UI::Views