#pragma once

#include <Views/ViewBase.h>

namespace Kub3::UI::Views
{

    class ExposureViewBase : public ViewBase
    {
        Q_OBJECT

    public:
        explicit ExposureViewBase(Unique<ViewModels::BaseViewModel> viewModel, QWidget *parent = nullptr);

        virtual ~ExposureViewBase() = default;

    private:
        void configTitleBar() override;

    protected:
        void setDefaultTitleBar(const QString &viewTitle);
        void onHomeButtonClicked(const QString &buttonId);
        void createNavButtonsConfigs() final override;
        virtual void onValidateButtonClicked(const QString &buttonId) = 0;
    };

} // namespace Kub3::UI::Views