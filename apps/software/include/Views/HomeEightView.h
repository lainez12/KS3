#ifndef HOMEEIGHTVIEW_H
#define HOMEEIGHTVIEW_H

#include <QWidget>

#include <ViewModels/HomeViewModel.h>
#include <Views/Components/NavButton.h>

#include "ViewBase.h"

namespace Ui
{
    class HomeEightView;
}

namespace Kub3::UI::Views
{

    class HomeEightView final : public ViewBase
    {
        using HomeViewModel = Kub3::UI::ViewModels::HomeViewModel;

        Q_OBJECT

    public:
        explicit HomeEightView(Unique<HomeViewModel> viewModel, QWidget *parent = nullptr);
        ~HomeEightView();

    signals:

    public slots:
        void onExposureMenuClicked();
        void onMaskAlignmentClicked();
    private slots:

    public:
        void resizeEvent(QResizeEvent *event) override;

    private:
        void updateMachineLogo(int h);
        void createNavButtonsConfigs();
        void configTitleBar();

        void onSettingsButtonClicked(const QString &buttonId);

    private:
        Ui::HomeEightView *ui;
    };
}

using HomeEightView = Kub3::UI::Views::HomeEightView;

#endif // HOMEVIEW_H
