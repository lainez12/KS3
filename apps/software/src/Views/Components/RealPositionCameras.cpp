#include "Views/Components/RealPositionCameras.h"
#include "ui_RealPositionCameras.h"

#include <QWidget>

RealPositionCameras::RealPositionCameras(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealPositionCameras)
{
    ui->setupUi(this);
    // ui->backgroundWafer->hide();
    ui->backgroundWafer->setVisible(false);

    connect(ui->btnOpenClose, &QCheckBox::toggled, this, &RealPositionCameras::onBtnOpenCloseToggled);
}

void RealPositionCameras::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
}

void RealPositionCameras::openMap(void)
{
    ui->btnOpenClose->setChecked(true);
    ui->backgroundWafer->setVisible(true);
}

void RealPositionCameras::closeMap(void)
{
    ui->btnOpenClose->setChecked(false);
    ui->backgroundWafer->setVisible(false);
}

void RealPositionCameras::onBtnOpenCloseToggled(bool checked)
{
    if (checked)
    {
        ui->backgroundWafer->setVisible(true);
        emit s_openMap();
    }
    else
    {
        ui->backgroundWafer->setVisible(false);
        emit s_closeMap();
    }
}