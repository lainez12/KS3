#pragma once

#include <QWidget>

#include <ViewModels/HomeViewModel.h>
#include <Views/Components/NavButton.h>

#include "ViewBase.h"

namespace Ui
{
    class HomeView;
}

namespace Kub3::UI::Views
{

    class HomeView final : public ViewBase
    {
        using HomeViewModel = ViewModels::HomeViewModel;

        Q_OBJECT

    public:
        explicit HomeView(Unique<HomeViewModel> viewModel, QWidget *parent = nullptr);
        ~HomeView();

    public slots:
        void ps_setDrawerActionsLock(bool lock);
        void ps_setExposureModeLock(bool lock);
        void ps_setHomingLock(bool lock);
        void ps_setInitializationLock(bool lock);

    private slots:
        void onExposureMenuClicked();
        void onMaskAlignmentClicked();
        void onInitializationClicked();
        void onWaferInsertBtnClicked();
        void onMaskInsertBtnClicked();
        void onWaferEjectBtnClicked();
        void onMaskEjectBtnClicked();

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void updateMachineLogo(int h);
        void createNavButtonsConfigs();
        void configTitleBar();

        void onSettingsButtonClicked();

    private:
        Ui::HomeView *ui;
    };
}

using HomeView = Kub3::UI::Views::HomeView;
