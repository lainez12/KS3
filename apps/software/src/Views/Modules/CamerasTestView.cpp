#include "ui_CamerasTestView.h"

#include <HAL/Vision/identifiers.h>
#include <ViewModels/Settings/MachineStatusViewModel.h>
#include <Views/Components/DoubleRatioSlider.h>
#include <Views/Modules/CamerasTestView.h>

namespace
{
    struct FramerateItem {
        const char *text;
        int value;
    };

    const FramerateItem __framerates[] = {
        {"24 Hz", 24},
        {"25 Hz", 25},
        {"30 Hz", 30},
        {"48 Hz", 48},
        {"50 Hz", 50},
        {"60 Hz", 60},
    };

    struct CenteredZoomItem {
        const char *text;
        double value;
    };

    const CenteredZoomItem __centeredZooms[] = {
        {"1x Zoom (Full Sensor, 100%)", 1.0},
        {"2x Zoom (Center 50%)", 2.0},
        {"4x Zoom (Center 25%)", 4.0},
    };
}

namespace Kub3::UI::Modules
{

    CamerasTestView::CamerasTestView(ViewModels::MachineStatusViewModel *viewModel, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::CamerasTestView),
        m_viewModel(viewModel)
    {
        ui->setupUi(this);

        setWindowModality(Qt::ApplicationModal); // Force application modal behavior
        setMinimumSize(800, 480);                // Ensure the modal has a decent minimum size

        // Frame reception
        connect(m_viewModel, &ViewModels::MachineStatusViewModel::s_frameReady, this,
                [this](const QString &cameraId, const QImage &frame) {
                    if (cameraId == UPPER_LEFT_CAMERA)
                        ui->leftCameraWidget->ps_onFrameUpdated(frame);
                    else if (cameraId == UPPER_RIGHT_CAMERA)
                        ui->rightCameraWidget->ps_onFrameUpdated(frame);
                });

        setupExposureComponents();
        setupGainComponents();
        setupFramerateComponents();
        setupCenteredZoomComponents();
        setupRegionOfInterestComponents();
    }

    CamerasTestView::~CamerasTestView()
    {
        delete ui;
    }

    void CamerasTestView::sendROIUpdate(
        const char *camId,
        QSpinBox *xBox,
        QSpinBox *yBox,
        QSpinBox *wBox,
        QSpinBox *hBox)
    {
        if (!xBox || !yBox || !wBox || !hBox)
            return;

        emit m_viewModel->s_roiChanged(camId, QRect(xBox->value(), yBox->value(), wBox->value(), hBox->value()));
    }

    void CamerasTestView::setupExposureComponents(void)
    {
        connect(
            ui->upperLeftCamExposureSlider, &DoubleRatioSlider::doubleValueChanged,
            [vm = m_viewModel](double val) { emit vm->s_exposureSliderValueChanged(UPPER_LEFT_CAMERA, val); });
        connect(
            ui->upperRightCamExposureSlider, &DoubleRatioSlider::doubleValueChanged,
            [vm = m_viewModel](double val) { emit vm->s_exposureSliderValueChanged(UPPER_RIGHT_CAMERA, val); });
    }

    void CamerasTestView::setupGainComponents(void)
    {
        connect(
            ui->upperLeftCamGainSlider, &DoubleRatioSlider::doubleValueChanged,
            [vm = m_viewModel](double val) { emit vm->s_gainSliderValueChanged(UPPER_LEFT_CAMERA, val); });
        connect(
            ui->upperRightCamGainSlider, &DoubleRatioSlider::doubleValueChanged,
            [vm = m_viewModel](double val) { emit vm->s_gainSliderValueChanged(UPPER_RIGHT_CAMERA, val); });
    }

    void CamerasTestView::setupFramerateComponents(void)
    {
        for (const auto &it : __framerates)
        {
            ui->upperLeftCamFramerateBox->addItem(QString::fromUtf8(it.text), QVariant(it.value));
            ui->upperRightCamFramerateBox->addItem(QString::fromUtf8(it.text), QVariant(it.value));
        }
        ui->upperLeftCamFramerateBox->setCurrentIndex(0);
        ui->upperRightCamFramerateBox->setCurrentIndex(0);

        connect(
            ui->upperLeftCamFramerateBox, &QComboBox::currentIndexChanged,
            [vm = m_viewModel, box = ui->upperLeftCamFramerateBox](int idx) {
                emit vm->s_framerateValueChanged(UPPER_LEFT_CAMERA, box->itemData(idx).toDouble());
            });
        connect(
            ui->upperRightCamFramerateBox, &QComboBox::currentIndexChanged,
            [vm = m_viewModel, box = ui->upperRightCamFramerateBox](int idx) {
                emit vm->s_framerateValueChanged(UPPER_RIGHT_CAMERA, box->itemData(idx).toDouble());
            });
    }

    void CamerasTestView::setupCenteredZoomComponents(void)
    {
        for (const auto &it : __centeredZooms)
        {
            ui->upperLeftCameraCZoomBox->addItem(QString::fromUtf8(it.text), QVariant(it.value));
            ui->upperRightCameraCZoomBox->addItem(QString::fromUtf8(it.text), QVariant(it.value));
        }
        ui->upperLeftCameraCZoomBox->setCurrentIndex(0);
        ui->upperRightCameraCZoomBox->setCurrentIndex(0);

        connect(
            ui->upperLeftCameraCZoomBox, &QComboBox::currentIndexChanged,
            [vm = m_viewModel, box = ui->upperLeftCameraCZoomBox](int idx) {
                emit vm->s_centeredZoomValueChanged(UPPER_LEFT_CAMERA, box->itemData(idx).toDouble());
            });
        connect(
            ui->upperRightCameraCZoomBox, &QComboBox::currentIndexChanged,
            [vm = m_viewModel, box = ui->upperRightCameraCZoomBox](int idx) {
                emit vm->s_centeredZoomValueChanged(UPPER_RIGHT_CAMERA, box->itemData(idx).toDouble());
            });
    }

    void CamerasTestView::setupRegionOfInterestComponents(void)
    {
        connect(ui->upperLeftCameraXBox, &QSpinBox::valueChanged, this, &CamerasTestView::onUpperLeftROIFieldChanged);
        connect(ui->upperLeftCameraYBox, &QSpinBox::valueChanged, this, &CamerasTestView::onUpperLeftROIFieldChanged);
        connect(ui->upperLeftCameraWidthBox, &QSpinBox::valueChanged, this, &CamerasTestView::onUpperLeftROIFieldChanged);
        connect(ui->upperLeftCameraHeightBox, &QSpinBox::valueChanged, this, &CamerasTestView::onUpperLeftROIFieldChanged);
        connect(ui->upperRightCameraXBox, &QSpinBox::valueChanged, this, &CamerasTestView::onUpperRightROIFieldChanged);
        connect(ui->upperRightCameraYBox, &QSpinBox::valueChanged, this, &CamerasTestView::onUpperRightROIFieldChanged);
        connect(ui->upperRightCameraWidthBox, &QSpinBox::valueChanged, this, &CamerasTestView::onUpperRightROIFieldChanged);
        connect(ui->upperRightCameraHeightBox, &QSpinBox::valueChanged, this, &CamerasTestView::onUpperRightROIFieldChanged);
    }

    void CamerasTestView::onUpperLeftROIFieldChanged(void)
    {
        sendROIUpdate(
            UPPER_LEFT_CAMERA,
            ui->upperLeftCameraXBox,
            ui->upperLeftCameraYBox,
            ui->upperLeftCameraWidthBox,
            ui->upperLeftCameraHeightBox);
    }

    void CamerasTestView::onUpperRightROIFieldChanged(void)
    {
        sendROIUpdate(
            UPPER_RIGHT_CAMERA,
            ui->upperRightCameraXBox,
            ui->upperRightCameraYBox,
            ui->upperRightCameraWidthBox,
            ui->upperRightCameraHeightBox);
    }

}
