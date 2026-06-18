#ifndef COMPLETEEXPOSUREVIEW_H
#define COMPLETEEXPOSUREVIEW_H

#include <QWidget>
#include <ViewModels/Exposure/CompleteExposureViewModel.h>

#include <Views/ExposureViewBase.h>

namespace Ui
{
    class CompleteExposureView;
} // namespace UI

namespace Kub3::UI::Views::ViewsExposure
{
    class CompleteExposureView final : public ExposureViewBase
    {
        using CompleteExposureViewModel = Kub3::UI::ViewModels::Exposure::CompleteExposureViewModel;

        Q_OBJECT

    public:
        explicit CompleteExposureView(Unique<CompleteExposureViewModel> viewModel, QWidget *parent = nullptr);
        ~CompleteExposureView();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void setNewNavButtonsConfigs();
        void onBackButtonClicked(const QString &buttonId);
        void onValidateButtonClicked(const QString &buttonId) override;

    private:
        Ui::CompleteExposureView *ui;
    };

} // namespace Kub3::UI::Views

using CompleteExposureView = Kub3::UI::Views::ViewsExposure::CompleteExposureView;

#endif