#pragma once

#include <QWidget>

#include <ViewModels/ExposureMenuViewModel.h>
#include <Views/Components/NavButton.h>

#include "ViewBase.h"

namespace Ui
{
    class ExposureMenuView;
}

namespace Kub3::UI::Views
{

    class ExposureMenuView final : public ViewBase
    {
        using ExposureMenuViewModel = Kub3::UI::ViewModels::ExposureMenuViewModel;

        Q_OBJECT

    public:
        explicit ExposureMenuView(Unique<ExposureMenuViewModel> viewModel, QWidget *parent = nullptr);
        ~ExposureMenuView();

    private:
        void createNavButtonsConfigs();
        void configTitleBar();

    private slots:
        void onBackButtonClicked();

    private:
        Ui::ExposureMenuView *ui;
    };
}

using ExposureMenuView = Kub3::UI::Views::ExposureMenuView;
