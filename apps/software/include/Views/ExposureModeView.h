#ifndef EXPOSUREMODEVIEW_H
#define EXPOSUREMODEVIEW_H

#include <QWidget>

#include <ViewModels/HomeViewModel.h>
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
        using HomeViewModel = Kub3::UI::ViewModels::HomeViewModel;

        Q_OBJECT

    public:
        explicit ExposureModeView(Unique<HomeViewModel> viewModel, QWidget *parent = nullptr);
        ~ExposureModeView();

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

        void onBackButtonClicked(const QString &buttonId);

    private:
        Ui::ExposureModeView *ui;
    };
}

using ExposureModeView = Kub3::UI::Views::ExposureModeView;

#endif // EXPOSUREMODEVIEW_H
