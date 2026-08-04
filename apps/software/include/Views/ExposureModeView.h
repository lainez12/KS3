#pragma once

#include <QWidget>

#include <ViewModels/ExposureModeViewModel.h>
#include <Views/Components/NavButton.h>

#include "ViewBase.h"

namespace Ui
{
    class ExposureModeView;
}

namespace Kub3::UI::Views
{

    class ExposureModeView final : public ViewBase
    {
        using ExposureModeViewModel = Kub3::UI::ViewModels::ExposureModeViewModel;

        Q_OBJECT

    public:
        explicit ExposureModeView(Unique<ExposureModeViewModel> viewModel, QWidget *parent = nullptr);
        ~ExposureModeView();

    public slots:
        void ps_onFloodExposureClicked();
        void ps_onMaskAlignmentClicked();

    private slots:
        void setAlignmentViewLock(bool lock);
        void setFloodExposureLock(bool lock);
        void onPreparingAlignment();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void updateMachineLogo(int h);
        void createNavButtonsConfigs() override;
        void configTitleBar() override;

        void onBackButtonClicked();
        void onBackToMainButtonClicked();

    private:
        Ui::ExposureModeView *ui;
    };
}

using ExposureModeView = Kub3::UI::Views::ExposureModeView;
