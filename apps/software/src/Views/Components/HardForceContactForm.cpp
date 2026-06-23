#include "Views/Components/HardForceContactForm.h"
#include "ui_HardForceContactForm.h"

#include <QWidget>

HardForceContactForm::HardForceContactForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HardForceContactForm)
{
    ui->setupUi(this);
}

void HardForceContactForm::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
}