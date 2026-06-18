#pragma once

#include <QDialog>
#include <QSpinBox>
#include <memory>

#include <utils.h>

namespace Ui
{
    class CamerasTestView;
}

namespace Kub3::UI::ViewModels
{
    class MachineStatusViewModel;
}

namespace Kub3::UI::Modules
{

    class CamerasTestView : public QDialog
    {
        Q_OBJECT
    public:
        explicit CamerasTestView(ViewModels::MachineStatusViewModel *viewModel, QWidget *parent = nullptr);
        ~CamerasTestView() override;

    public slots:
        void onUpperLeftROIFieldChanged(void);
        void onUpperRightROIFieldChanged(void);

    private:
        void sendROIUpdate(const char *camId, QSpinBox *xBox, QSpinBox *yBox, QSpinBox *wBox, QSpinBox *hBox);
        // View setup functions
        void setupExposureComponents(void);
        void setupGainComponents(void);
        void setupFramerateComponents(void);
        void setupCenteredZoomComponents(void);
        void setupRegionOfInterestComponents(void);

    private:
        Ui::CamerasTestView *ui;
        ViewModels::MachineStatusViewModel *m_viewModel;
    };

}
