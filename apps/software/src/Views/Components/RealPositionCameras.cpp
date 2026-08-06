#include "Views/Components/RealPositionCameras.h"
#include "ui_RealPositionCameras.h"

#include <QWidget>

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

    ui->cameraLeftLabel->resize(ui->backgroundWafer->size() / 8);
    ui->cameraRightLabel->resize(ui->backgroundWafer->size() / 8);

    connect(ui->btnOpenClose, &QCheckBox::toggled, this, &RealPositionCameras::onBtnOpenCloseToggled);

    areaSize    = 236314;
    maskSize    = 228600;
    resizingMap = ui->backgroundWafer->size() * maskSize / areaSize;
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

void RealPositionCameras::updateLeftPosition(float x, float y)
{
    ui->cameraLeftLabel->move(((ui->backgroundWafer->width() - ui->cameraLeftLabel->width()) / 2) + (x * resizingMap.width() / maskSize) - 2, ((ui->backgroundWafer->height() - ui->cameraLeftLabel->height()) / 2) - (y * resizingMap.height() / maskSize));

    this->update();
}

void RealPositionCameras::updateRightPosition(float x, float y)
{
    ui->cameraRightLabel->move(((ui->backgroundWafer->width() - ui->cameraRightLabel->width()) / 2) + (x * resizingMap.width() / maskSize) + 2, ((ui->backgroundWafer->height() - ui->cameraRightLabel->height()) / 2) - (y * resizingMap.height() / maskSize));
    this->update();
}