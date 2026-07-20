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
    ui->setZeroBtn->setup(NavButton::SetupParams{"Set Zero", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/setZero.svg"});
    ui->gotoBtn->setup(NavButton::SetupParams{"Go to", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/go-to.svg"});
    ui->gramBtn->setup(NavButton::SetupParams{"Gram", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/gram.svg"});
    ui->newtonBtn->setup(NavButton::SetupParams{"Newton", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/newton.svg"});
}