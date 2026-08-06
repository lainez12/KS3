#include <QWidget>

#include <Views/Components/Colors.h>
#include <Views/Components/HardForceContactForm.h>

#include "ui_HardForceContactForm.h"

HardForceContactForm::HardForceContactForm(QWidget *parent) :
    QWidget(parent),
    m_keyboard(this),
    ui(new Ui::HardForceContactForm)
{
    ui->setupUi(this);
    m_keyboard.setupKeyboardConnections(this);
    ui->setZeroBtn->setup(NavButton::SetupParams{"Set Zero", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/setZero.svg"});
    ui->gotoBtn->setup(NavButton::SetupParams{"Go to", QColor(BLUE_COLOR), QColor(BLUE_COLOR_SHADOW), ":/icons/go-to.svg"});

    connect(ui->gotoBtn, &NavButton::clicked, this, [this]() {
        emit s_startContactRoutine(ui->sbTargetForce->value());
    });
}

void HardForceContactForm::ps_setMaximum(double maximumGF)
{
    const int processedValue = static_cast<int>(maximumGF);

    ui->sbTargetForce->setMaximum(processedValue);
    ui->lblMaxTargetGF->setText(QString("%1 grams-force").arg(processedValue));
}

void HardForceContactForm::ps_setTolerance(double toleranceGF)
{
    const int processedValue = static_cast<int>(toleranceGF);

    ui->sbTargetForce->setMinimum(processedValue);
    ui->lblToleranceGF->setText(QString("± %1 grams-force").arg(processedValue));
}

void HardForceContactForm::ps_setLock(bool lock)
{
    ui->sbTargetForce->setEnabled(!lock);
    ui->gotoBtn->setEnabled(!lock);
    ui->setZeroBtn->setEnabled(!lock);
    ui->kbContainer->setEnabled(!lock);
}
