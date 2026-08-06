#include "Views/Components/RealPositionCameras.h"
#include "ui_RealPositionCameras.h"
#include <Common/Enums.h>

#include <QWidget>

#define ICON_PATH_CAMERA ":/icons/rouge.svg"

RealPositionCameras::RealPositionCameras(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealPositionCameras),
    m_icon(QPixmap(":/icons/fleche-haut.svg"))
{
    ui->setupUi(this);
    ui->backgroundWafer->setVisible(false);
    ui->backgroundWafer->setStyleSheet(
        "background-image: url(:/icons/wafer6pouce.png);"
        "background-repeat: no-repeat;"
        "background-position: center;");

    QSize resizing(ui->backgroundWafer->size() / 8);
    ui->cameraLeftLabel->resize(resizing);
    ui->cameraRightLabel->resize(resizing);
    ui->cameraLeftLabel->setPixmap(QPixmap(ICON_PATH_CAMERA).scaled(resizing));
    ui->cameraRightLabel->setPixmap(QPixmap(ICON_PATH_CAMERA).scaled(resizing));

    connect(ui->btnOpenClose, &QCheckBox::toggled, this, &RealPositionCameras::onBtnOpenCloseToggled);

    // TODO: Find a better way to get the size of the mask and area, maybe from the config file or from the camera itself
    areaSize = 236314;
    maskSize = 228600;

    resizingMap = ui->backgroundWafer->size() * maskSize / areaSize;

    widthBackgroundWafer  = ui->backgroundWafer->width();
    heightBackgroundWafer = ui->backgroundWafer->height();
    widthCam              = ui->cameraRightLabel->width();
    heightCam             = ui->cameraRightLabel->height();
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
    ui->backgroundWafer->setFixedWidth(this->width());
}

void RealPositionCameras::onBtnOpenCloseToggled(bool checked)
{
    QTransform t;

    if (checked)
    {
        t.rotate(180);
        ui->backgroundWafer->setVisible(true);
        this->adjustSize();
        emit s_openMap();
    }
    else
    {
        ui->backgroundWafer->setVisible(false);
        this->adjustSize();
        emit s_closeMap();
    }

    auto rotatedIcon = m_icon.transformed(t);
    ui->btnOpenClose->setIcon(rotatedIcon);
}

void RealPositionCameras::updateRightXPosition(double x)
{
    double newX = ((widthBackgroundWafer - widthCam) / 2) + (x * resizingMap.width() / maskSize) + 2;
    double y    = ui->cameraRightLabel->y();
    ui->cameraRightLabel->move(newX, y);
    this->update();
}

void RealPositionCameras::updateRightYPosition(double y)
{
    double x    = ui->cameraRightLabel->x();
    double newY = ((heightBackgroundWafer - heightCam) / 2) - (y * resizingMap.height() / maskSize);
    ui->cameraRightLabel->move(x, newY);
    this->update();
}

void RealPositionCameras::updateLeftXPosition(double x)
{
    double newX = ((widthBackgroundWafer - widthCam) / 2) + (x * resizingMap.width() / maskSize) + 2;
    double y    = ui->cameraLeftLabel->y();
    ui->cameraLeftLabel->move(newX, y);
    this->update();
}

void RealPositionCameras::updateLeftYPosition(double y)
{
    double x    = ui->cameraLeftLabel->x();
    double newY = ((heightBackgroundWafer - heightCam) / 2) - (y * resizingMap.height() / maskSize);
    ui->cameraLeftLabel->move(x, newY);
    this->update();
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