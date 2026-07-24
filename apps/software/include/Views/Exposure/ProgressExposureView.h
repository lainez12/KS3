#ifndef PROGRESSEXPOSUREVIEW_H
#define PROGRESSEXPOSUREVIEW_H

#include <QTimer>
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
        void showEvent(QShowEvent *event) override;

    private:
        void onBackButtonClicked();
        void onValidateButtonClicked() override;
        void updateProgressBar();

    private:
        Ui::ProgressExposureView *ui;
        QTimer m_progressTimer;
        QElapsedTimer m_elapsedTimer;
        uint32_t m_durationInMS = 0;
    };

} // namespace Kub3::UI::Views

using ProgressExposureView = Kub3::UI::Views::ViewsExposure::ProgressExposureView;

#endif