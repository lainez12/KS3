#ifndef HOMEVIEW_H
#define HOMEVIEW_H

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
        using HomeViewModel = Kub3::UI::ViewModels::HomeViewModel;

        Q_OBJECT

    public:
        explicit HomeView(Unique<HomeViewModel> viewModel, QWidget *parent = nullptr);
        ~HomeView();

    signals:

    public slots:
        void onFloodExposureClicked();
        void onMaskAlignmentClicked();
    private slots:

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void updateMachineLogo(int h);
        void createNavButtonsConfigs();
        void configTitleBar();

        void onSettingsButtonClicked(const QString &buttonId);
        void onOpenButtonClicked(const QString &buttonId);
        void onCloseButtonCliked(const QString &buttonId);

    private:
        Ui::HomeView *ui;
    };
}

using HomeView = Kub3::UI::Views::HomeView;

#endif // HOMEVIEW_H
