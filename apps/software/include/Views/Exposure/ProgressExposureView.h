#ifndef PROGRESSEXPOSUREVIEW_H
#define PROGRESSEXPOSUREVIEW_H

#include <QWidget>
#include <ViewModels/Exposure/ProgressExposureViewModel.h>

#include <Views/ExposureViewBase.h>

namespace Ui
{
    class ProgressExposureView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure
{
    class ProgressExposureView final : public ExposureViewBase
    {
        using ProgressExposureViewModel = Kub3::UI::ViewModels::Exposure::ProgressExposureViewModel;

        Q_OBJECT

    public:
        explicit ProgressExposureView(Unique<ProgressExposureViewModel> viewModel, QWidget *parent = nullptr);
        ~ProgressExposureView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void onBackButtonClicked();
        void onValidateButtonClicked() override;

    private:
        Ui::ProgressExposureView *ui;
    };

} // namespace Kub3::UI::Views

using ProgressExposureView = Kub3::UI::Views::ViewsExposure::ProgressExposureView;

#endif