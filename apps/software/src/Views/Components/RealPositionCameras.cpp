#include "Views/Components/RealPositionCameras.h"
#include "ui_RealPositionCameras.h"

#include <QWidget>

RealPositionCameras::RealPositionCameras(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RealPositionCameras)
{
    ui->setupUi(this);
}

void RealPositionCameras::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
}