#include "ui_CamerasTestView.h"
#include <ViewModels/MachineStatusViewModel.h>
#include <Views/Modules/CamerasTestView.h>

namespace Kub3::UI::Modules
{

    CamerasTestView::CamerasTestView(std::shared_ptr<ViewModels::MachineStatusViewModel> viewModel, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::CamerasTestView),
        m_viewModel(std::move(viewModel))
    {
        ui->setupUi(this);

        setWindowModality(Qt::ApplicationModal); // Force application modal behavior
        setMinimumSize(800, 480);                // Ensure the modal has a decent minimum size

        connect(
            m_viewModel.get(),
            &ViewModels::MachineStatusViewModel::s_upperLeftCameraFrameReady,
            ui->leftCameraWidget,
            &CameraStreamWidget::ps_onFrameUpdated);

        connect(
            m_viewModel.get(),
            &ViewModels::MachineStatusViewModel::s_upperRightCameraFrameReady,
            ui->rightCameraWidget,
            &CameraStreamWidget::ps_onFrameUpdated);
    }

    CamerasTestView::~CamerasTestView()
    {
        delete ui;
    }

}
