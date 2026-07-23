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

    public:
        void showEvent(QShowEvent *event) override;

    protected:
        // Populate
        void createNavButtonsConfigs() final override;
        void setDefaultTitleBar(const QString &viewTitle);

    protected slots:
        // Buttons
        void onHomeButtonClicked();
        virtual void onValidateButtonClicked() = 0;

    private:
        void configTitleBar() override;
    };

} // namespace Kub3::UI::Views

using PresetExposure = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::PresetExposure;
using ExposureMode   = Kub3::UI::ViewModels::Exposure::ExposureBaseViewModel::ExposureMode;