#include "Views/Components/HardForceContactForm.h"
#include "ui_HardForceContactForm.h"
#include <Views/Components/Colors.h>

#include <QWidget>

HardForceContactForm::HardForceContactForm(QWidget *parent) :
    QWidget(parent),
    m_keyboard(this),
    ui(new Ui::HardForceContactForm)
{
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this);
    ui->setZeroBtn->setup("Set Zero", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/setZero.svg");
    ui->gotoBtn->setup("Go to", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/go-to.svg");
    ui->gramBtn->setup("Gram", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/gram.svg");
    ui->newtonBtn->setup("Newton", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/newton.svg");
}