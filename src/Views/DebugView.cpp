#include "Views/DebugView.h"
#include "ui_DebugView.h"

DebugView::DebugView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DebugView)
{
    ui->setupUi(this);
}

void DebugView::updateMachineState(const QString &stateName)
{
    ui->MFSMStateLabel->setText(stateName);
}

void DebugView::on_openMachineStatusViewBtn_clicked(void)
{
    emit s_openMachineStatus();
}

void DebugView::on_startInitializationBtn_clicked(void)
{
    emit s_initializationRequest();
}

DebugView::~DebugView()
{
    delete ui;
}
