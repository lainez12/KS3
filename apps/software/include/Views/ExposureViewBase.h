#pragma once

#include <ViewModels/Exposure/ExposureBaseViewModel.h>
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
        void onHomeButtonClicked();
        void createNavButtonsConfigs() final override;
        virtual void onValidateButtonClicked() = 0;
    };

} // namespace Kub3::UI::Views

using PresetExposure = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::PresetExposure;
using ExposureMode   = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::ExposureMode;