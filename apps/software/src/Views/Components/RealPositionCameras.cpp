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