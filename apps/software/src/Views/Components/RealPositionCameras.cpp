#include <Common/Enums.h>
#include <Views/Components/RealPositionCameras.h>
#include <ui_RealPositionCameras.h>

#include <QWidget>
#include <cmath>

#define ICON_PATH_CAMERA ":/icons/peepholered.png"

RealPositionCameras::RealPositionCameras(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealPositionCameras),
    m_icon(QPixmap(":/icons/fleche-haut.svg"))
{
    ui->setupUi(this);

    // Align playground centrally
    ui->verticalLayout->setAlignment(ui->playground, Qt::AlignCenter);
    ui->playground->setVisible(false);

    // scaledContents property is True in UI; QLabel will scale it automatically
    ui->cameraLeftLabel->setPixmap(QPixmap(ICON_PATH_CAMERA));
    ui->cameraRightLabel->setPixmap(QPixmap(ICON_PATH_CAMERA));

    connect(ui->btnOpenClose, &QCheckBox::toggled, this, &RealPositionCameras::onBtnOpenCloseToggled);
}

RealPositionCameras::~RealPositionCameras()
{
    delete ui;
}

void RealPositionCameras::openMap(void)
{
    ui->btnOpenClose->setChecked(true);
}

void RealPositionCameras::closeMap(void)
{
    ui->btnOpenClose->setChecked(false);
}

void RealPositionCameras::resizeEvent(QResizeEvent *ev)
{
    QWidget::resizeEvent(ev);

    // Calculate available space
    const int playgroundWidth  = ui->btnContainer->width();
    const int playgroundHeight = playgroundWidth * 0.8;

    ui->playground->setFixedSize(playgroundWidth, playgroundHeight);

    // Wafer takes exactly 95% of the playground space
    int waferSide = static_cast<int>(playgroundHeight * 0.85);
    int waferX    = (playgroundWidth - waferSide) / 2;
    int waferY    = (playgroundHeight - waferSide) / 2;
    ui->lblWafer200mm->setGeometry(waferX, waferY, waferSide, waferSide);

    // Camera icons are exactly wafer length / 8
    int camSide = waferSide / 8;
    ui->cameraLeftLabel->resize(camSide, camSide);
    ui->cameraRightLabel->resize(camSide, camSide);

    updateCameraPositions();
}

void RealPositionCameras::onBtnOpenCloseToggled(bool checked)
{
    QTransform t;

    if (checked)
    {
        t.rotate(180);
        ui->playground->setVisible(true);
        this->adjustSize();
        emit s_openMap();
    }
    else
    {
        ui->playground->setVisible(false);
        this->adjustSize();
        emit s_closeMap();
    }

    auto rotatedIcon = m_icon.transformed(t);
    ui->btnOpenClose->setIcon(rotatedIcon);
}

void RealPositionCameras::updateCameraPositions()
{
    if (ui->lblWafer200mm->width() <= 0)
        return;

    // Coordinate (0, 0) is perfectly centered on the lblWafer200mm
    double waferCenterX = ui->lblWafer200mm->x() + ui->lblWafer200mm->width() / 2.0;
    double waferCenterY = ui->lblWafer200mm->y() + ui->lblWafer200mm->height() / 2.0;

    // The entire Wafer is 200mm wide/high in the coordinate system
    double pixelsPerMm = ui->lblWafer200mm->width() / 200.0;

    // Standard mappings: +x travels right, +y travels Up (negating UI standard +y down structure)
    double leftCx = waferCenterX + (m_leftX * pixelsPerMm);
    double leftCy = waferCenterY - (m_leftY * pixelsPerMm);

    double rightCx = waferCenterX + (m_rightX * pixelsPerMm);
    double rightCy = waferCenterY - (m_rightY * pixelsPerMm);

    // Update positions mapping the camera center to corresponding logical locations
    ui->cameraLeftLabel->move(static_cast<int>(std::round(leftCx - ui->cameraLeftLabel->width() / 2.0)),
                              static_cast<int>(std::round(leftCy - ui->cameraLeftLabel->height() / 2.0)));

    ui->cameraRightLabel->move(static_cast<int>(std::round(rightCx - ui->cameraRightLabel->width() / 2.0)),
                               static_cast<int>(std::round(rightCy - ui->cameraRightLabel->height() / 2.0)));

    this->update();
}

void RealPositionCameras::updateRightXPosition(double x)
{
    m_rightX = x;
    updateCameraPositions();
}

void RealPositionCameras::updateRightYPosition(double y)
{
    m_rightY = y;
    updateCameraPositions();
}

void RealPositionCameras::updateLeftXPosition(double x)
{
    m_leftX = x;
    updateCameraPositions();
}

void RealPositionCameras::updateLeftYPosition(double y)
{
    m_leftY = y;
    updateCameraPositions();
}

void RealPositionCameras::onCameraPositionUpdate(Kub3::CameraId camId, Kub3::CameraAxis axis, double value)
{
    if (camId == Kub3::CameraId::LEFT)
    {
        if (axis == Kub3::CameraAxis::X)
            updateLeftXPosition(value);
        else if (axis == Kub3::CameraAxis::Y)
            updateLeftYPosition(value);
    }
    else if (camId == Kub3::CameraId::RIGHT)
    {
        if (axis == Kub3::CameraAxis::X)
            updateRightXPosition(value);
        else if (axis == Kub3::CameraAxis::Y)
            updateRightYPosition(value);
    }
}
